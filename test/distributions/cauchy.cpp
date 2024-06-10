/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <numbers>
#include <openrng.h>
#include <refng_vsl.h>
#include <utils/distribution_templates.hpp>
#include <utils/golden/cauchy.hpp>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinULP;

namespace {
template <std::floating_point T> const T normalisedAbsErrorBound;
template <> const auto normalisedAbsErrorBound<double> = pow(10, -7);
template <> const auto normalisedAbsErrorBound<float> = powf(10, -4);

template <std::floating_point T> const T normalRelErrorBound;
template <> const auto normalRelErrorBound<double> = pow(10, -10);
template <> const auto normalRelErrorBound<float> = powf(10, -4);

template <std::floating_point T> const T extremeRelErrorBound;
template <> const auto extremeRelErrorBound<double> = pow(10, -10);
template <> const auto extremeRelErrorBound<float> = 0.02;

template <std::floating_point T> T cauchyIcdf(T u, T a, T beta) {
  return beta * tan((u - 0.5) * std::numbers::pi) - a;
}

/**
 * Tolerace for WithinAbs.
 *
 * This tolerance is used for checking values around 0. We know the error grows
 * with a and beta, so the abosulte tolerance is dependent on the input
 * parameters.
 *
 * One situation this method doesn't handle is if the value is both in the tail,
 * and near zero. This scenario can happen if the mean is so large that 0 is in
 * the extreme regions. We avoid testing it to avoid further complicating an
 * already complicated test, but we could handle it by introducing an extreme
 * absolute error bound similar to how we handle relative error.
 */
template <std::floating_point T> T getCauchyAbsoluteTolerance(T a, T beta) {
  return std::max(std::abs(a), std::abs(beta)) * normalisedAbsErrorBound<T>;
}

/**
 * Tolerance for WithinRel.
 *
 * For Cauchy, we see the relative error around the tails can be quite large, so
 * to avoid introducing a sizeable relative error everywhere, we divide the
 * testing into three regions. The left and right tails, and everywhere else. If
 * in the tails, we use a a looser relative tolerance.
 */
template <std::floating_point T>
T getCauchyRelativeTolerance(T result, T a, T beta) {
  const T extremeRegion = 0.001;
  const T lowerExtreme = cauchyIcdf<T>(extremeRegion, a, beta);
  const T upperExtreme = cauchyIcdf<T>(1 - extremeRegion, a, beta);

  if (result > lowerExtreme && result < upperExtreme) {
    return normalRelErrorBound<T>;
  } else {
    return extremeRelErrorBound<T>;
  }
}

} // namespace

TEMPLATE_TEST_CASE("Cauchy expected data", "[icdf]", float, double) {
  const int acceptedULP = 128;
  CAPTURE(acceptedULP);

  for (auto [key, reference] : CauchyGolden::reference<TestType>()) {
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
    errcode = rRngCauchy<TestType>(method, rstream, n, buffer.data(), a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    for (int i = 0; i < n; i++) {
      REQUIRE_THAT(reference[i], WithinULP(buffer[i], acceptedULP));
    }

    errcode = refDeleteStream(&rstream);

    REQUIRE(errcode == VSL_ERROR_OK);
  }
}

TEMPLATE_TEST_CASE(
    "Cauchy ICDF returns the same series when called iteratively", "[icdf]",
    float, double) {

  const auto n = 100;
  const auto method = VSL_RNG_METHOD_CAUCHY_ICDF;
  const auto a = GENERATE(as<TestType>{}, 0, 1, -1, 2, -2, 123.2);
  const auto beta = GENERATE(as<TestType>{}, 1, 2, 123.2);
  const TestType absErrorBound = getCauchyAbsoluteTolerance(a, beta);

  VSLStreamStatePtr streamVector;
  VSLStreamStatePtr streamScalar;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&streamVector, VSL_BRNG_MCG31, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&streamScalar, VSL_BRNG_MCG31, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  TestType buffer[n];
  errcode = vRngCauchy<TestType>(method, streamVector, n, buffer, a, beta);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < n; i++) {
    TestType scalar;
    errcode = vRngCauchy<TestType>(method, streamScalar, 1, &scalar, a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);
    REQUIRE_THAT(buffer[i], WithinAbs(scalar, absErrorBound));
  }

  errcode = vslDeleteStream(&streamVector);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&streamScalar);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Cauchy ICDF is approximately comparable to refng", "[icdf]",
                   float, double) {
  const int valuesToGenerate = 600;
  const auto skipAhead = GENERATE(0, 1, 2, 3, 100, 1000);
  const auto a = GENERATE(as<TestType>{}, 0, 1, -1, 2, -2, 123.2);
  const auto beta = GENERATE(as<TestType>{}, 1, 2, 123.2);
  const auto bufferSize = GENERATE_COPY(1, 2, 3, 100, valuesToGenerate);
  const auto seed = GENERATE(as<int>{}, 0, 1, 0x1234);
  const auto absErrorBound = getCauchyAbsoluteTolerance(a, beta);

  CAPTURE(valuesToGenerate, skipAhead, a, beta, bufferSize, seed);

  VSLStreamStatePtr vStream, rStream;
  int errcode;
  errcode = vslNewStream(&vStream, VSL_BRNG_MCG31, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rStream, VSL_BRNG_MCG31, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  // We test generation of valuesToGenerate numbers, either by filling a buffer
  // of size valuesToGenerate or smaller. To keep the testing simple, we
  // require valuesToGenerate to be divisible by bufferSize.
  REQUIRE((valuesToGenerate / bufferSize) * bufferSize == valuesToGenerate);

  errcode = vslSkipAheadStream(vStream, skipAhead);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refSkipAheadStream(rStream, skipAhead);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < valuesToGenerate; i += bufferSize) {
    std::vector<TestType> vslBuffer(bufferSize);
    errcode = vRngCauchy<TestType>(VSL_RNG_METHOD_CAUCHY_ICDF, vStream,
                                   vslBuffer.size(), vslBuffer.data(), a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<TestType> refBuffer(bufferSize);
    errcode = rRngCauchy<TestType>(VSL_RNG_METHOD_CAUCHY_ICDF, rStream,
                                   refBuffer.size(), refBuffer.data(), a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    for (int i = 0; i < vslBuffer.size(); i++) {
      CAPTURE(i);

      //
      // Cauchy is similar to a Gaussian distribution, except it has "fat
      // tails", that is, it approaches +/- infinity much faster than a Gaussian
      // distribution so we consider the relative error to bound values >> 1.
      //
      const TestType relErrorBound =
          getCauchyRelativeTolerance(vslBuffer[i], a, beta);

      REQUIRE_THAT(vslBuffer[i], (WithinAbs(refBuffer[i], absErrorBound) ||
                                  WithinRel(refBuffer[i], relErrorBound)));
    }
  }
  errcode = vslDeleteStream(&vStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Cauchy ICDF bad arguments", "[icdf]", float, double) {

  auto n = 100;
  TestType a = 0;
  TestType beta = 1;
  TestType buffer[n];

  VSLStreamStatePtr stream;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_R250, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode = vRngCauchy<TestType>(0xffff, stream, n, buffer, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream address)") {
    errcode = vRngCauchy<TestType>(VSL_RNG_METHOD_CAUCHY_ICDF, nullptr, n,
                                   buffer, a, beta);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (the size)") {
    errcode = vRngCauchy<TestType>(VSL_RNG_METHOD_CAUCHY_ICDF, stream, -1,
                                   buffer, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (the buffer)") {
    errcode = vRngCauchy<TestType>(VSL_RNG_METHOD_CAUCHY_ICDF, stream, n,
                                   nullptr, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the sixth parameter (beta)") {
    errcode = vRngCauchy<TestType>(VSL_RNG_METHOD_CAUCHY_ICDF, stream, n,
                                   buffer, a, -1);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}