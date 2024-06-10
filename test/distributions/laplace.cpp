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
#include <utils/golden/laplace.hpp>

using Catch::Matchers::WithinAbs, Catch::Matchers::WithinULP;

namespace {
template <typename T> const T normalisedAbsErrorBound;
template <> const auto normalisedAbsErrorBound<double> = pow(10, -7);
template <> const auto normalisedAbsErrorBound<float> = powf(10, -4);

template <typename T> T getLaplaceErrBound(T a, T beta) {
  return std::abs(beta * normalisedAbsErrorBound<T>);
}
} // namespace

TEMPLATE_TEST_CASE("Laplace expected data", "[icdf]", float, double) {
  const int acceptedULP = 1 << sizeof(TestType); // float -> 16 & double -> 256
  CAPTURE(acceptedULP);

  for (auto [key, reference] : LaplaceGolden::reference<TestType>()) {
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
    errcode = rRngLaplace<TestType>(method, rstream, n, buffer.data(), a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    for (int i = 0; i < n; i++) {
      REQUIRE_THAT(reference[i], WithinULP(buffer[i], acceptedULP));
    }

    errcode = refDeleteStream(&rstream);

    REQUIRE(errcode == VSL_ERROR_OK);
  }
}

TEMPLATE_TEST_CASE(
    "Laplace ICDF returns the same series when called iteratively", "[icdf]",
    float, double) {

  const auto n = 100;
  const auto method = VSL_RNG_METHOD_LAPLACE_ICDF;
  const auto a = GENERATE(as<TestType>{}, 0, 1, -1, 2, -2, 123.2);
  const auto beta = GENERATE(as<TestType>{}, 1, 2, 123.2);
  const TestType absErrorBound = getLaplaceErrBound(a, beta);

  VSLStreamStatePtr streamVector;
  VSLStreamStatePtr streamScalar;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&streamVector, VSL_BRNG_MCG31, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&streamScalar, VSL_BRNG_MCG31, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  TestType buffer[n];
  errcode = vRngLaplace<TestType>(method, streamVector, n, buffer, a, beta);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < n; i++) {
    TestType scalar;
    errcode = vRngLaplace<TestType>(method, streamScalar, 1, &scalar, a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);
    REQUIRE_THAT(buffer[i], WithinAbs(scalar, absErrorBound));
  }

  errcode = vslDeleteStream(&streamVector);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&streamScalar);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Laplace ICDF is approximately comparable to refng",
                   "[icdf]", float, double) {
  const int valuesToGenerate = 600;
  const auto skipAhead = GENERATE(0, 1, 2, 3, 100, 1000);
  const auto a = GENERATE(as<TestType>{}, 0, 1, -1, 2, -2, 123.2);
  const auto beta = GENERATE(as<TestType>{}, 1, 2, 123.2);
  const auto bufferSize = GENERATE_COPY(1, 2, 3, 100, valuesToGenerate);
  const int seed = GENERATE(0, 1, 0x1234);
  const TestType absErrorBound = getLaplaceErrBound(a, beta);

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
    errcode =
        vRngLaplace<TestType>(VSL_RNG_METHOD_LAPLACE_ICDF, vStream,
                              vslBuffer.size(), vslBuffer.data(), a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<TestType> refBuffer(bufferSize);
    errcode =
        rRngLaplace<TestType>(VSL_RNG_METHOD_LAPLACE_ICDF, rStream,
                              refBuffer.size(), refBuffer.data(), a, beta);
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

TEMPLATE_TEST_CASE("Laplace ICDF bad arguments", "[icdf]", float, double) {

  auto n = 100;
  TestType a = 0;
  TestType beta = 1;
  TestType buffer[n];

  VSLStreamStatePtr stream;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_R250, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode = vRngLaplace<TestType>(0xffff, stream, n, buffer, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream address)") {
    errcode = vRngLaplace<TestType>(VSL_RNG_METHOD_LAPLACE_ICDF, nullptr, n,
                                    buffer, a, beta);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (the size)") {
    errcode = vRngLaplace<TestType>(VSL_RNG_METHOD_LAPLACE_ICDF, stream, -1,
                                    buffer, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (the buffer)") {
    errcode = vRngLaplace<TestType>(VSL_RNG_METHOD_LAPLACE_ICDF, stream, n,
                                    nullptr, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the sixth parameter (beta)") {
    auto bad_beta = GENERATE(-1, 0);
    CAPTURE(bad_beta);
    errcode = vRngLaplace<TestType>(VSL_RNG_METHOD_LAPLACE_ICDF, stream, n,
                                    buffer, a, bad_beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
