/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
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
#include <utils/golden/weibull.hpp>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinULP;

namespace {
template <typename T> const T normalisedAbsErrorBound;
template <> const auto normalisedAbsErrorBound<double> = pow(10, -7);
template <> const auto normalisedAbsErrorBound<float> = powf(10, -4);

template <typename T> T getWeibullErrBound(T a, T beta) {
  return std::abs(beta * normalisedAbsErrorBound<T>);
}
} // namespace

TEMPLATE_TEST_CASE("Weibull expected data", "[weibull][icdf]", float, double) {
  const int acceptedULP = 2'000'000;
  CAPTURE(acceptedULP);

  for (auto [key, reference] : WeibullGolden::reference<TestType>()) {
    const int n = reference.size();
    const TestType alpha = key.alpha;
    const TestType a = key.a;
    const TestType beta = key.beta;
    const int seed = key.seed;
    const auto brngId = key.brngId;
    const auto method = key.method;
    CAPTURE(n, brngId, method, seed, alpha, a, beta);

    VSLStreamStatePtr rstream;
    int errcode = refNewStream(&rstream, brngId, seed);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<TestType> buffer(n);
    errcode = rRngWeibull<TestType>(method, rstream, n, buffer.data(), alpha, a,
                                    beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    for (int i = 0; i < n; i++) {
      REQUIRE_THAT(reference[i], WithinULP(buffer[i], acceptedULP));
    }

    errcode = refDeleteStream(&rstream);

    REQUIRE(errcode == VSL_ERROR_OK);
  }
}

TEMPLATE_TEST_CASE("Weibull duplicate stream", "[weibull][icdf]", float,
                   double) {

  const auto n = 100;
  const auto method = GENERATE(VSL_RNG_METHOD_WEIBULL_ICDF,
                               VSL_RNG_METHOD_WEIBULL_ICDF_ACCURATE);
  const auto a = GENERATE(as<TestType>{}, 0, 1, -1, 2, -2, 123.2);
  const auto beta = GENERATE(as<TestType>{}, 1, 2, 123.2);
  const auto alpha = GENERATE(as<TestType>{}, 1, 3, 8, 123.2);
  const TestType absErrorBound = getWeibullErrBound(a, beta);

  CAPTURE(method, a, beta, alpha);

  VSLStreamStatePtr streamVector;
  VSLStreamStatePtr streamScalar;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&streamVector, VSL_BRNG_MCG31, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&streamScalar, VSL_BRNG_MCG31, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  TestType buffer[n];
  errcode =
      vRngWeibull<TestType>(method, streamVector, n, buffer, alpha, a, beta);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < n; i++) {
    TestType scalar;
    errcode =
        vRngWeibull<TestType>(method, streamScalar, 1, &scalar, alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);
    REQUIRE_THAT(buffer[i], WithinAbs(scalar, absErrorBound));
  }

  errcode = vslDeleteStream(&streamVector);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&streamScalar);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Weibull reference test", "[weibull][icdf]", float, double) {
  const int valuesToGenerate = 600;
  const auto skipAhead = GENERATE(0, 1, 2, 3, 100, 1000);
  const auto a = GENERATE(as<TestType>{}, 0, 1, -1, 2, -2, 123.2);
  const auto beta = GENERATE(as<TestType>{}, 1, 2, 123.2);
  const auto alpha = GENERATE(as<TestType>{}, 1, 3, 8, 123.2);
  const auto bufferSize = GENERATE_COPY(1, 2, 3, 100, valuesToGenerate);
  const int seed = GENERATE(0, 1, 0x1234);
  const TestType absErrorBound = getWeibullErrBound(a, beta);

  const auto method = GENERATE(VSL_RNG_METHOD_WEIBULL_ICDF,
                               VSL_RNG_METHOD_WEIBULL_ICDF_ACCURATE);

  CAPTURE(valuesToGenerate, skipAhead, a, beta, bufferSize, seed, method);

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
    errcode = vRngWeibull<TestType>(VSL_RNG_METHOD_WEIBULL_ICDF, vStream,
                                    vslBuffer.size(), vslBuffer.data(), alpha,
                                    a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<TestType> refBuffer(bufferSize);
    errcode = rRngWeibull<TestType>(VSL_RNG_METHOD_WEIBULL_ICDF, rStream,
                                    refBuffer.size(), refBuffer.data(), alpha,
                                    a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    for (int i = 0; i < vslBuffer.size(); i++) {
      REQUIRE_THAT(vslBuffer[i], WithinAbs(refBuffer[i], absErrorBound));
    }
  }
  errcode = vslDeleteStream(&vStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Weibull bad arguments", "[weibull][icdf]", float, double) {

  auto n = 100;
  TestType alpha = 2;
  TestType a = 4;
  TestType beta = 6;
  TestType buffer[n];

  const auto method = GENERATE(VSL_RNG_METHOD_WEIBULL_ICDF,
                               VSL_RNG_METHOD_WEIBULL_ICDF_ACCURATE);

  VSLStreamStatePtr stream;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_R250, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode = vRngWeibull<TestType>(0xffff, stream, n, buffer, alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream address)") {
    errcode = vRngWeibull<TestType>(method, nullptr, n, buffer, alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (the size)") {
    errcode = vRngWeibull<TestType>(method, stream, -1, buffer, alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (the buffer)") {
    errcode = vRngWeibull<TestType>(method, stream, n, nullptr, alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fifth parameter (alpha)") {
    const TestType bad_alpha = GENERATE(-1, 0);
    CAPTURE(bad_alpha);
    errcode =
        vRngWeibull<TestType>(method, stream, n, buffer, bad_alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the seventh parameter (beta)") {
    const TestType bad_beta = GENERATE(-1, 0);
    CAPTURE(bad_beta);
    errcode =
        vRngWeibull<TestType>(method, stream, n, buffer, alpha, a, bad_beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}