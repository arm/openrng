/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <openrng.h>
#include <refng_vsl.h>
#include <utils/distribution_templates.hpp>
#include <utils/golden/chisquare.hpp>

#include <cmath>
#include <vector>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinULP;

namespace {
template <typename T> const T normalisedAbsErrorBound;
template <> const auto normalisedAbsErrorBound<double> = std::pow(10, -7);
template <> const auto normalisedAbsErrorBound<float> = std::pow(10, -4);

template <typename T> T getChiSquareErrBound(T v) {
  return std::abs(v * normalisedAbsErrorBound<T>);
}
} // namespace

namespace {
template <typename T> T calculateMean(const std::vector<T> &buffer) {
  const int n = static_cast<int>(buffer.size());
  T mean = T(0);
  for (int i = 0; i < n; i++) {
    mean += buffer[i];
  }

  return mean / static_cast<T>(n);
}
} // namespace

namespace {
template <typename T>
T calculateVariance(const std::vector<T> &buffer, T mean) {
  const int n = static_cast<int>(buffer.size());
  T var = T(0);
  for (int i = 0; i < n; i++) {
    const T d = buffer[i] - mean;
    var += d * d;
  }
  return var / (static_cast<T>(n) - 1);
}
} // namespace

TEMPLATE_TEST_CASE("ChiSquare bad arguments", "[chisquare]", float, double) {
  const auto n = 1000;
  const int v = 5;
  TestType buffer[n];

  const auto method = VSL_RNG_METHOD_CHISQUARE_CHI2GAMMA;
  CAPTURE(method);

  VSLStreamStatePtr stream;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_MT19937, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode = vRngChiSquare<TestType>(0xffff, stream, n, buffer, v);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream address)") {
    errcode = vRngChiSquare<TestType>(method, nullptr, n, buffer, v);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (the size)") {
    errcode = vRngChiSquare<TestType>(method, stream, -1, buffer, v);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (the buffer)") {
    errcode = vRngChiSquare<TestType>(method, stream, n, nullptr, v);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fifth parameter (v)") {
    TestType bad_v = GENERATE(0, -1);
    CAPTURE(bad_v);
    errcode = vRngChiSquare<TestType>(method, stream, n, buffer, bad_v);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("ChiSquare all values >=0 ", "[chisquare]", float, double) {

  const int n = GENERATE(100, 1000, 10000);
  // These values are to select all code paths in the implementation.
  const int v = GENERATE(1, 2, 3, 7, 10, 18);
  const int seed = GENERATE(0, 1, 0x123);
  TestType refBuffer[n];
  TestType vslBuffer[n];

  const auto method = VSL_RNG_METHOD_CHISQUARE_CHI2GAMMA;
  CAPTURE(method, n, v, seed);

  VSLStreamStatePtr rstream;
  auto errcode = refNewStream(&rstream, VSL_BRNG_MT19937, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  VSLStreamStatePtr vstream;
  errcode = vslNewStream(&vstream, VSL_BRNG_MT19937, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vRngChiSquare<TestType>(method, vstream, n, vslBuffer, v);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = rRngChiSquare<TestType>(method, rstream, n, refBuffer, v);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < n; i++) {
    REQUIRE(vslBuffer[i] >= 0);
    REQUIRE(refBuffer[i] >= 0);
  }

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("ChiSquare Expected Mean", "[chisquare]", float, double) {

  const int n = GENERATE(20000, 50000);
  const int seed = GENERATE(0, 1, 0x123);
  const int v = GENERATE(1, 2, 3, 7, 10, 18);
  std::vector<TestType> refBuffer(n);
  std::vector<TestType> vslBuffer(n);

  const auto method = VSL_RNG_METHOD_CHISQUARE_CHI2GAMMA;
  CAPTURE(method, n, v, seed);

  VSLStreamStatePtr rstream;
  auto errcode = refNewStream(&rstream, VSL_BRNG_MT19937, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  VSLStreamStatePtr vstream;
  errcode = vslNewStream(&vstream, VSL_BRNG_MT19937, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vRngChiSquare<TestType>(method, vstream, n, vslBuffer.data(), v);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = rRngChiSquare<TestType>(method, rstream, n, refBuffer.data(), v);
  REQUIRE(errcode == VSL_ERROR_OK);

  const TestType allowedRelativeError = static_cast<TestType>(0.02);

  const TestType refSampleMean = calculateMean(refBuffer);
  const TestType vslSampleMean = calculateMean(vslBuffer);

  const TestType predictedMean = static_cast<TestType>(v);
  CAPTURE(refSampleMean, vslSampleMean, predictedMean);
  REQUIRE_THAT(refSampleMean, WithinRel(predictedMean, allowedRelativeError));
  REQUIRE_THAT(vslSampleMean, WithinRel(predictedMean, allowedRelativeError));

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE(
    "ChiSquare returns values comparable to refng for v = 1 and v = 3",
    "[chisquare]", float, double) {

  const int n = GENERATE(10, 100, 2500);
  const int seed = GENERATE(0, 1, 0x123);
  const int v = GENERATE(1, 3);
  std::vector<TestType> refBuffer(n);
  std::vector<TestType> vslBuffer(n);

  const auto method = VSL_RNG_METHOD_CHISQUARE_CHI2GAMMA;
  CAPTURE(method, n, v, seed);

  VSLStreamStatePtr rstream;
  auto errcode = refNewStream(&rstream, VSL_BRNG_MT19937, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  VSLStreamStatePtr vstream;
  errcode = vslNewStream(&vstream, VSL_BRNG_MT19937, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vRngChiSquare<TestType>(method, vstream, n, vslBuffer.data(), v);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = rRngChiSquare<TestType>(method, rstream, n, refBuffer.data(), v);
  REQUIRE(errcode == VSL_ERROR_OK);

  const TestType allowedAbsoluteError =
      getChiSquareErrBound(static_cast<TestType>(v));

  for (int i = 0; i < n; i++) {
    REQUIRE_THAT(refBuffer[i], WithinAbs(vslBuffer[i], allowedAbsoluteError));
  }

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("ChiSquare returns values comparable to refng for v = 2 and "
                   "any even number <=16",
                   "[chisquare]", float, double) {
  const int n = GENERATE(20000, 50000);
  const int seed = GENERATE(0, 1, 0x123);
  const int v = GENERATE(2, 4, 6, 8, 10, 12, 14, 16);
  std::vector<TestType> refBuffer(n);
  std::vector<TestType> vslBuffer(n);

  const auto method = VSL_RNG_METHOD_CHISQUARE_CHI2GAMMA;
  CAPTURE(method, n, v, seed);

  VSLStreamStatePtr rstream;
  auto errcode = refNewStream(&rstream, VSL_BRNG_MT19937, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  VSLStreamStatePtr vstream;
  errcode = vslNewStream(&vstream, VSL_BRNG_MT19937, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vRngChiSquare<TestType>(method, vstream, n, vslBuffer.data(), v);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = rRngChiSquare<TestType>(method, rstream, n, refBuffer.data(), v);
  REQUIRE(errcode == VSL_ERROR_OK);

  const TestType allowedAbsoluteError =
      getChiSquareErrBound(static_cast<TestType>(v));

  for (int i = 0; i < n; i++) {
    REQUIRE_THAT(refBuffer[i], WithinAbs(vslBuffer[i], allowedAbsoluteError));
  }

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("ChiSquare returns values comparable to refng for odd v >=5 "
                   "or v >=17",
                   "[chisquare][openrng_only]", float, double) {
  const int n = GENERATE(20000, 50000);
  const int seed = GENERATE(0, 1, 0x123);
  const int v = GENERATE(5, 7, 9, 11, 13, 15, 17, 19, 20);
  std::vector<TestType> refBuffer(n);
  std::vector<TestType> vslBuffer(n);

  const auto method = VSL_RNG_METHOD_CHISQUARE_CHI2GAMMA;
  CAPTURE(method, n, v, seed);

  VSLStreamStatePtr rstream;
  auto errcode = refNewStream(&rstream, VSL_BRNG_MT19937, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  VSLStreamStatePtr vstream;
  errcode = vslNewStream(&vstream, VSL_BRNG_MT19937, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vRngChiSquare<TestType>(method, vstream, n, vslBuffer.data(), v);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = rRngChiSquare<TestType>(method, rstream, n, refBuffer.data(), v);
  REQUIRE(errcode == VSL_ERROR_OK);

  const TestType allowedAbsoluteError =
      getChiSquareErrBound(static_cast<TestType>(v));

  for (int i = 0; i < n; i++) {
    REQUIRE_THAT(refBuffer[i], WithinAbs(vslBuffer[i], allowedAbsoluteError));
  }

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("ChiSquare Expected Variance ", "[chisquare]", float,
                   double) {

  const int n = GENERATE(20000, 50000);
  const int seed = GENERATE(0, 1, 0x123);
  const int v = GENERATE(1, 2, 3, 7, 10, 18);
  std::vector<TestType> refBuffer(n);
  std::vector<TestType> vslBuffer(n);

  const auto method = VSL_RNG_METHOD_CHISQUARE_CHI2GAMMA;
  CAPTURE(method, n, v, seed);

  VSLStreamStatePtr rstream;
  auto errcode = refNewStream(&rstream, VSL_BRNG_MT19937, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  VSLStreamStatePtr vstream;
  errcode = vslNewStream(&vstream, VSL_BRNG_MT19937, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vRngChiSquare<TestType>(method, vstream, n, vslBuffer.data(), v);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = rRngChiSquare<TestType>(method, rstream, n, refBuffer.data(), v);
  REQUIRE(errcode == VSL_ERROR_OK);

  const TestType allowedRelativeError = static_cast<TestType>(0.05);

  const TestType refSampleVariance =
      calculateVariance(refBuffer, static_cast<TestType>(v));
  const TestType vslSampleVariance =
      calculateVariance(vslBuffer, static_cast<TestType>(v));

  const TestType predictedVariance = static_cast<TestType>(2 * v);
  CAPTURE(refSampleVariance, vslSampleVariance, predictedVariance);
  REQUIRE_THAT(refSampleVariance,
               WithinRel(predictedVariance, allowedRelativeError));
  REQUIRE_THAT(vslSampleVariance,
               WithinRel(predictedVariance, allowedRelativeError));

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("ChiSquare expected data", "[chisquare][openrng_only]",
                   float, double) {
  const int acceptedULP = 1 << sizeof(TestType); // float -> 16 & double -> 256
  CAPTURE(acceptedULP);

  for (auto [key, reference] : ChiSquareGolden::reference<TestType>()) {
    const int n = reference.size();
    const TestType v = key.v;
    const int seed = key.seed;
    const auto brngId = key.brngId;
    const auto method = key.method;
    CAPTURE(n, brngId, method, seed, v);

    VSLStreamStatePtr rstream;
    int errcode = refNewStream(&rstream, brngId, seed);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<TestType> buffer(n);
    errcode = rRngChiSquare<TestType>(method, rstream, n, buffer.data(), v);
    REQUIRE(errcode == VSL_ERROR_OK);

    for (int i = 0; i < n; i++) {
      REQUIRE_THAT(reference[i], WithinULP(buffer[i], acceptedULP));
    }

    errcode = refDeleteStream(&rstream);
    REQUIRE(errcode == VSL_ERROR_OK);
  }
}