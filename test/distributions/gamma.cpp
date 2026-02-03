/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
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
#include <fstream>
#include <iomanip>
#include <openrng.h>
#include <refng_vsl.h>
#include <stdexcept>
#include <string>
#include <utils/distribution_templates.hpp>
#include <utils/golden/gamma.hpp>
#include <utils/names_map.hpp>
#include <vector>
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinULP;

namespace {
template <typename T> const T normalisedAbsErrorBound;
template <> const auto normalisedAbsErrorBound<double> = pow(10, -7);
template <> const auto normalisedAbsErrorBound<float> = pow(10, -4);

template <typename T> T getGammaErrBound(T beta) {
  return std::abs(beta * normalisedAbsErrorBound<T>);
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
  return var / static_cast<T>(n);
}
} // namespace

TEMPLATE_TEST_CASE("Gamma output has approximately expected mean", "[gamma]",
                   float, double) {
  const int n = 100'000;
  const uint32_t seed = GENERATE(0, 1, 0x123);
  const auto method = GENERATE(VSL_RNG_METHOD_GAMMA_GNORM);

  const auto params =
      GENERATE(std::make_tuple<TestType, TestType, TestType>(5, 0, 1),
               std::make_tuple<TestType, TestType, TestType>(1, 0, 1),
               std::make_tuple<TestType, TestType, TestType>(2, 0, 5),
               std::make_tuple<TestType, TestType, TestType>(2.567, 5, 2),
               std::make_tuple<TestType, TestType, TestType>(339, 2, 1),
               std::make_tuple<TestType, TestType, TestType>(45.5, 20, 3),
               std::make_tuple<TestType, TestType, TestType>(3, 0, 0.001),
               std::make_tuple<TestType, TestType, TestType>(0.0001, 2, 0.0001),
               std::make_tuple<TestType, TestType, TestType>(0.001, -2, 0.001),
               std::make_tuple<TestType, TestType, TestType>(0.01, 2, 0.01),
               std::make_tuple<TestType, TestType, TestType>(0.1, -2, 0.1),
               std::make_tuple<TestType, TestType, TestType>(0.5, -46.2, 0.5),
               std::make_tuple<TestType, TestType, TestType>(0.315, 0, 3),
               std::make_tuple<TestType, TestType, TestType>(0.599, 0.2, 30),
               std::make_tuple<TestType, TestType, TestType>(0.741, 0.5, 1.89),
               std::make_tuple<TestType, TestType, TestType>(0.943, 0.2, 12));

  auto [alpha, a, beta] = params;

  const TestType predictedMean = a + alpha * beta;
  const TestType allowedRelativeError = static_cast<TestType>(0.05);
  CAPTURE(n, alpha, a, beta, seed, method, allowedRelativeError);

  const auto brngId = VSL_BRNG_MT19937;
  CAPTURE(method, brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<TestType> buffer(n);

  errcode =
      vRngGamma<TestType>(method, stream, n, buffer.data(), alpha, a, beta);
  REQUIRE(errcode == VSL_ERROR_OK);

  const TestType mean = calculateMean(buffer);
  CAPTURE(mean, predictedMean);
  REQUIRE_THAT(mean, WithinRel(predictedMean, allowedRelativeError));

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Gamma output has approximately expected variance",
                   "[gamma]", float, double) {
  const int n = 100'000;
  const std::uint32_t seed = GENERATE(0, 1, 0x123);
  const auto method = GENERATE(VSL_RNG_METHOD_GAMMA_GNORM);

  const auto params =
      GENERATE(std::make_tuple<TestType, TestType, TestType>(14, 0, 1),
               std::make_tuple<TestType, TestType, TestType>(1, 0, 1),
               std::make_tuple<TestType, TestType, TestType>(2.56, 2, 4),
               std::make_tuple<TestType, TestType, TestType>(15, 5, 2),
               std::make_tuple<TestType, TestType, TestType>(3, 0, 1),
               std::make_tuple<TestType, TestType, TestType>(1.3, 20, 3),
               std::make_tuple<TestType, TestType, TestType>(3, 0, 0.001),
               std::make_tuple<TestType, TestType, TestType>(0.1, 0.36, 0.1),
               std::make_tuple<TestType, TestType, TestType>(0.5, -2, 0.5),
               std::make_tuple<TestType, TestType, TestType>(0.315, 0, 3),
               std::make_tuple<TestType, TestType, TestType>(0.599, 0.2, 30),
               std::make_tuple<TestType, TestType, TestType>(0.741, 0.5, 1.89),
               std::make_tuple<TestType, TestType, TestType>(0.943, 0.2, 12));

  TestType alpha, a, beta;
  std::tie(alpha, a, beta) = params;

  const TestType predictedVariance = alpha * beta * beta;
  const TestType allowedRelativeError = static_cast<TestType>(0.08);
  CAPTURE(n, a, alpha, beta, seed, method, predictedVariance,
          allowedRelativeError);

  const auto brngId = VSL_BRNG_MT19937;
  CAPTURE(method, brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<TestType> buffer(n);
  errcode =
      vRngGamma<TestType>(method, stream, n, buffer.data(), alpha, a, beta);
  REQUIRE(errcode == VSL_ERROR_OK);

  const TestType mean = calculateMean(buffer);
  const TestType variance = calculateVariance(buffer, mean);

  CAPTURE(mean, variance, predictedVariance);
  REQUIRE_THAT(variance, WithinRel(predictedVariance, allowedRelativeError));

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Gamma GNorm is approximately comparable to refng",
                   "[openrng_only]", float, double) {
  const int valuesToGenerate = 600;
  const auto skipAhead = GENERATE(0, 1, 2, 3, 100, 1000);
  const auto a = GENERATE(as<TestType>{}, -1, -2, 0, 1, 2, 123.2);
  const auto alpha = GENERATE(as<TestType>{}, 0.0001, 0.001, 0.01, 0.1, 0.5,
                              0.31756, 1, 2, 123.2);
  const auto beta = GENERATE(as<TestType>{}, 0.1, 0.5, 1, 2, 123.2);
  const auto bufferSize = GENERATE_COPY(1, 2, 3, 100, valuesToGenerate);
  const int seed = GENERATE(0, 1, 0x1234);
  const TestType absErrorBound = getGammaErrBound(beta);

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

  for (int i = 0; i < valuesToGenerate; i += bufferSize) {
    std::vector<TestType> vslBuffer(bufferSize);
    errcode =
        vRngGamma<TestType>(VSL_RNG_METHOD_GAMMA_GNORM, vStream,
                            vslBuffer.size(), vslBuffer.data(), alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<TestType> refBuffer(bufferSize);
    errcode =
        rRngGamma<TestType>(VSL_RNG_METHOD_GAMMA_GNORM, rStream,
                            refBuffer.size(), refBuffer.data(), alpha, a, beta);
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

TEMPLATE_TEST_CASE(
    "Gamma GNorm has same numbers for refng and vsl when alpha >= 0.6 && < 1",
    "[gnorm]", float, double) {
  const int valuesToGenerate = 600;
  const auto skipAhead = GENERATE(0, 1, 2, 3, 100, 1000);
  const auto a = GENERATE(as<TestType>{}, -1, -2, 0, 1, 2, 123.2);
  const auto alpha = GENERATE(as<TestType>{}, 0.6012, 0.753, 0.99);
  const auto beta = GENERATE(as<TestType>{}, 0.1, 0.5, 1, 2, 123.2);
  const auto bufferSize = GENERATE_COPY(1, 2, 3, 100, valuesToGenerate);
  const int seed = GENERATE(0, 1, 0x1234);
  const TestType absErrorBound = getGammaErrBound(beta);

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

  for (int i = 0; i < valuesToGenerate; i += bufferSize) {
    std::vector<TestType> vslBuffer(bufferSize);
    errcode =
        vRngGamma<TestType>(VSL_RNG_METHOD_GAMMA_GNORM, vStream,
                            vslBuffer.size(), vslBuffer.data(), alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<TestType> refBuffer(bufferSize);
    errcode =
        rRngGamma<TestType>(VSL_RNG_METHOD_GAMMA_GNORM, rStream,
                            refBuffer.size(), refBuffer.data(), alpha, a, beta);
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

TEMPLATE_TEST_CASE("Gamma expected data", "[gamma][openrng_only]", float,
                   double) {
  const int acceptedULP = 1 << sizeof(TestType);
  CAPTURE(acceptedULP);

  for (auto [key, reference] : GammaGolden::reference<TestType>()) {
    const int n = reference.size();
    const TestType a = key.a;
    const TestType alpha = key.alpha;
    const TestType beta = key.beta;
    const int seed = key.seed;
    const auto brngId = key.brngId;
    const auto method = key.method;
    CAPTURE(n, brngId, method, seed, alpha, a, beta);

    VSLStreamStatePtr rstream;
    int errcode = refNewStream(&rstream, brngId, seed);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<TestType> buffer(n);
    errcode =
        rRngGamma<TestType>(method, rstream, n, buffer.data(), alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    for (int i = 0; i < n; i++) {
      REQUIRE_THAT(buffer[i],
                   WithinULP(reference[i], static_cast<uint64_t>(acceptedULP)));
    }

    errcode = refDeleteStream(&rstream);
    REQUIRE(errcode == VSL_ERROR_OK);
  }
}

TEMPLATE_TEST_CASE("Gamma bad arguments", "[gamma][openrng_only]", float,
                   double) {
  auto n = 1000;
  TestType a = 5;
  TestType alpha = 2;
  TestType beta = 1;
  TestType buffer[n];

  const auto method = GENERATE(VSL_RNG_METHOD_GAMMA_GNORM);
  CAPTURE(method);

  VSLStreamStatePtr stream;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_MT19937, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode = vRngGamma<TestType>(0xffff, stream, n, buffer, alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream address)") {
    errcode = vRngGamma<TestType>(method, nullptr, n, buffer, alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (the size)") {
    errcode = vRngGamma<TestType>(method, stream, -1, buffer, alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (the buffer)") {
    errcode = vRngGamma<TestType>(method, stream, n, nullptr, alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fifth parameter (alpha)") {
    TestType bad_alpha = GENERATE(0, -1);
    CAPTURE(bad_alpha);
    errcode =
        vRngGamma<TestType>(method, stream, n, buffer, bad_alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the seventh parameter (beta)") {
    TestType bad_beta = GENERATE(0, -1);
    CAPTURE(bad_beta);
    errcode =
        vRngGamma<TestType>(method, stream, n, buffer, alpha, a, bad_beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Gamma GNORM produces values greater than a", "[gamma]",
                   float, double) {
  const int valuesToGenerate = 600;
  const auto a = GENERATE(as<TestType>{}, 0, 1, -1, 2, -2, 123.2);
  const auto alpha =
      GENERATE(as<TestType>{}, 0.01, 0.378, 0.599, 0.62, 0.943, 1, 3, 7.5);
  const auto beta = GENERATE(as<TestType>{}, 0.5, 1, 2, 123.2);
  const auto bufferSize = GENERATE(1, 2, 4, 5, 10, 20);
  const int seed = GENERATE(0, 1, 0x1234);

  VSLStreamStatePtr vStream = nullptr;
  int errcode = vslNewStream(&vStream, VSL_BRNG_MCG31, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < valuesToGenerate; i += bufferSize) {
    std::vector<TestType> vslBuffer(bufferSize);
    errcode =
        vRngGamma<TestType>(VSL_RNG_METHOD_GAMMA_GNORM, vStream,
                            vslBuffer.size(), vslBuffer.data(), alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    // Check that all elements are strictly greater than a
    for (size_t j = 0; j < vslBuffer.size(); ++j) {
      REQUIRE(vslBuffer[j] >= a);
    }
  }

  errcode = vslDeleteStream(&vStream);
}

TEMPLATE_TEST_CASE("Gamma GNORM generates the same numbers as Exponential ICDF",
                   "[icdf]", float, double) {
  const int valuesToGenerate = 600;
  const auto a = GENERATE(as<TestType>{}, 0, 1, -1, 2, -2, 123.2);
  const auto alpha = GENERATE(as<TestType>{}, 1);
  const auto beta = GENERATE(as<TestType>{}, 0.001, 1, 2, 123.2);
  const auto bufferSize = GENERATE_COPY(1, 2, 4, 5, 10, 20);
  const int seed = GENERATE(0, 1, 0x1234);

  CAPTURE(valuesToGenerate, a, alpha, beta, bufferSize, seed);

  VSLStreamStatePtr gammaStream, expoStream;
  int errcode;
  errcode = vslNewStream(&gammaStream, VSL_BRNG_MCG31, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&expoStream, VSL_BRNG_MCG31, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE((valuesToGenerate / bufferSize) * bufferSize == valuesToGenerate);

  for (int i = 0; i < valuesToGenerate; i += bufferSize) {
    std::vector<TestType> vslBufferGamma(bufferSize);
    errcode = vRngGamma<TestType>(VSL_RNG_METHOD_GAMMA_GNORM, gammaStream,
                                  vslBufferGamma.size(), vslBufferGamma.data(),
                                  alpha, a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<TestType> vslBufferExpo(bufferSize);
    errcode = vRngExponential<TestType>(VSL_RNG_METHOD_EXPONENTIAL_ICDF,
                                        expoStream, vslBufferExpo.size(),
                                        vslBufferExpo.data(), a, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    REQUIRE(vslBufferExpo == vslBufferGamma);
  }
  errcode = vslDeleteStream(&gammaStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&expoStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
