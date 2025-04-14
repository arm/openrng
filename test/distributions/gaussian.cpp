/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <openrng.h>
#include <refng_vsl.h>
#include <utils/distribution_templates.hpp>
#include <utils/golden/gaussian.hpp>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinULP;

namespace {

template <typename T> const T normalisedAbsErrorBound;
template <> const auto normalisedAbsErrorBound<double> = pow(10, -7);
template <> const auto normalisedAbsErrorBound<float> = powf(10, -4);

/**
 * Get the empirical absolute error of a Gaussian distribution.
 *
 * A Gaussian variate, Y, is found by transforming a normally distributed
 * variate, X, by the following formula.
 *
 *   Y = sigma * X + mean.
 *
 * X is approximated using a transformation, f(U1, U2), where U1 and U2 are
 * known uniform variates, and f is a low precision function, especially in
 * double precision. The error in Y is relatively large due to catastrophic
 * cancellation when
 *
 *   sigma * X ~= mean
 *
 * for this reason, relative error is a bad choice for an upper bound. This
 * method returns an empirical upper bound for the absolute error in Y, that
 * scales with sigma.
 */
template <typename T> T getGaussianAbsErrorBound(T mean, T sigma) {
  return std::abs(sigma * normalisedAbsErrorBound<T>);
}

/**
 * Check whether a Gaussian-distributed value matches a
 * reference output.
 */
template <typename T>
void check(auto method, T vsl, T ref, T mean, T sigma, T tol) {
  // Basic checks: finite output, sign bits match
  REQUIRE(std::isfinite(vsl));
  REQUIRE(std::isfinite(ref));
  REQUIRE(std::signbit(vsl) == std::signbit(ref));

  // For the ICDF case, we ensure that we always return a finite value by
  // special casing inputs where erfinv(x) == +/- inf, i.e. x is +/- 1. Our
  // optimized implementation handles this differently to our reference, and we
  // accept relaxing our tests to just basic checks in these cases.
  if (method == VSL_RNG_METHOD_GAUSSIAN_ICDF) {
    const auto cdf = std::erf((ref - mean) / (std::sqrt(2) * sigma));
    const bool isExtreme = 1 - std::abs(cdf) < normalisedAbsErrorBound<T>;
    if (isExtreme)
      return;
  }

  REQUIRE_THAT(vsl, WithinAbs(ref, tol));
}

} // namespace

TEMPLATE_TEST_CASE("Gaussian expected data", "[gauss]", float, double) {
  const int acceptedULP = 64;
  CAPTURE(acceptedULP);

  for (auto [key, reference] : GaussianGolden::reference<TestType>()) {
    const int n = reference.size();
    const TestType a = key.a;
    const TestType beta = key.beta;
    const int seed = key.seed;
    const auto brngId = key.brngId;
    const auto method = key.method;
    CAPTURE(n, brngId, method, seed, a, beta);

    VSLStreamStatePtr rstream;
    int errcode = refNewStream(&rstream, brngId, seed);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<TestType> buffer(n);
    errcode =
        rRngGaussian<TestType>(method, rstream, n, buffer.data(), a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    for (int i = 0; i < n; i++) {
      REQUIRE_THAT(reference[i], WithinULP(buffer[i], acceptedULP));
    }

    errcode = refDeleteStream(&rstream);

    REQUIRE(errcode == VSL_ERROR_OK);
  }
}

TEMPLATE_TEST_CASE("Gaussian returns the same series when called iteratively",
                   "[gauss]", float, double) {

  const auto n = 100;
  const auto method =
      GENERATE(VSL_RNG_METHOD_GAUSSIAN_ICDF, VSL_RNG_METHOD_GAUSSIAN_BOXMULLER,
               VSL_RNG_METHOD_GAUSSIAN_BOXMULLER2);
  const auto mean = GENERATE(as<TestType>{}, 0, 1, -1, 2, -2, 123.2);
  const auto sigma = GENERATE(as<TestType>{}, 1, 2, 123.2);
  const TestType absErrorBound = getGaussianAbsErrorBound(mean, sigma);

  VSLStreamStatePtr streamVector;
  VSLStreamStatePtr streamScalar;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&streamVector, VSL_BRNG_MCG31, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&streamScalar, VSL_BRNG_MCG31, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  TestType buffer[n];
  errcode =
      vRngGaussian<TestType>(method, streamVector, n, buffer, mean, sigma);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < n; i++) {
    TestType scalar;
    errcode =
        vRngGaussian<TestType>(method, streamScalar, 1, &scalar, mean, sigma);
    REQUIRE(errcode == VSL_ERROR_OK);
    REQUIRE_THAT(buffer[i],
                 WithinAbs(scalar, getGaussianAbsErrorBound(mean, sigma)));
  }

  errcode = vslDeleteStream(&streamVector);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&streamScalar);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Gaussian is approximately comparable to refng", "[gauss]",
                   float, double) {
  const int valuesToGenerate = 600;
  const auto method =
      GENERATE(VSL_RNG_METHOD_GAUSSIAN_ICDF, VSL_RNG_METHOD_GAUSSIAN_BOXMULLER,
               VSL_RNG_METHOD_GAUSSIAN_BOXMULLER2);
  const auto skipAhead = GENERATE(0, 1, 2, 3, 100, 1000);
  const auto mean = GENERATE(as<TestType>{}, 0, 1, -1, 2, -2, 123.2);
  const auto sigma = GENERATE(as<TestType>{}, 1, 2, 123.2);
  const auto bufferSize = GENERATE_COPY(1, 2, 3, 100, valuesToGenerate);
  const TestType absErrorBound = getGaussianAbsErrorBound(mean, sigma);

  // One advantage of using the MCG31 generator is the first output is always
  // the seed, so we can test the generators extreme outputs by using them as
  // seeds, 0 and 2^31-2. This is particular useful for ICDF because it has
  // special casing for handling the singularities at the extremes.
  const int seed = GENERATE(0, 1, 0x1234, 0x7ffffffe);

  CAPTURE(method, valuesToGenerate, skipAhead, mean, sigma, bufferSize, seed);

  VSLStreamStatePtr vStream, rStream;
  int errcode;
  errcode = vslNewStream(&vStream, VSL_BRNG_MCG31, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rStream, VSL_BRNG_MCG31, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  // We test generation of valuesToGenerate numbers, either by filling a
  // buffer of size valuesToGenerate or smaller. To keep the testing simple,
  // we require valuesToGenerate to be divisible by bufferSize.
  REQUIRE((valuesToGenerate / bufferSize) * bufferSize == valuesToGenerate);

  errcode = vslSkipAheadStream(vStream, skipAhead);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refSkipAheadStream(rStream, skipAhead);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < valuesToGenerate; i += bufferSize) {
    std::vector<TestType> vslBuffer(bufferSize);
    errcode = vRngGaussian<TestType>(method, vStream, vslBuffer.size(),
                                     vslBuffer.data(), mean, sigma);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<TestType> refBuffer(bufferSize);
    errcode = rRngGaussian<TestType>(method, rStream, refBuffer.size(),
                                     refBuffer.data(), mean, sigma);
    REQUIRE(errcode == VSL_ERROR_OK);

    for (int i = 0; i < vslBuffer.size(); i++) {
      check(method, vslBuffer[i], refBuffer[i], mean, sigma, absErrorBound);
    }
  }

  errcode = vslDeleteStream(&vStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Gaussian bad arguments", "[icdf]", float, double) {

  auto n = 100;
  auto method = GENERATE(VSL_RNG_METHOD_GAUSSIAN_BOXMULLER,
                         VSL_RNG_METHOD_GAUSSIAN_BOXMULLER2,
                         VSL_RNG_METHOD_GAUSSIAN_ICDF);
  CAPTURE(method);

  TestType a = 0;
  TestType sigma = 1;
  TestType buffer[n];

  VSLStreamStatePtr stream;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_R250, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode = vRngGaussian<TestType>(0xffff, stream, n, buffer, a, sigma);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream address)") {
    errcode = vRngGaussian<TestType>(method, nullptr, n, buffer, a, sigma);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (the size)") {
    errcode = vRngGaussian<TestType>(method, stream, -1, buffer, a, sigma);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (the buffer)") {
    errcode = vRngGaussian<TestType>(method, stream, n, nullptr, a, sigma);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the sixth parameter (beta)") {
    errcode = vRngGaussian<TestType>(method, stream, n, buffer, a, -1);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
