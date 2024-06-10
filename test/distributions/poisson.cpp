/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <openrng.h>
#include <refng_vsl.h>
#include <utils/golden/poisson.hpp>
#include <utils/names_map.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;
using std::min_element, std::max_element;
using std::vector, std::pow;

namespace {
double calculateMean(const vector<int> &buffer) {
  const int n = buffer.size();
  double mean = 0.0;

  for (int i = 0; i < n; i++) {
    mean += buffer[i];
  }

  return mean / n;
}

double calculateVariance(const vector<int> &buffer) {
  const int n = buffer.size();
  const double mean = calculateMean(buffer);
  double variance = 0.0;

  for (int i = 0; i < n; i++) {
    variance += pow(buffer[i] - mean, 2);
  }

  return variance / (n - 1);
}

double calculateTruncatedPMF(const double lambda, vector<int> &bounds,
                             vector<double> &PMF) {
  assert(bounds.size() == 2);

  for (int k = bounds[0]; k <= bounds[1]; k++) {
    const double r =
        std::exp(k * std::log(lambda) - lambda - std::log(std::tgamma(k + 1)));
    PMF.push_back(r);
  }

  double r = 0.0;
  for (int k = 0; k < PMF.size(); k++) {
    r += PMF[k];
  }

  return r;
}

void transformToDistribution(const vector<int> &buffer,
                             const vector<int> &bounds, const double norm,
                             vector<double> &distribution) {
  const int n = buffer.size();

  for (int i = 0; i < n; i++) {
    distribution[buffer[i] - bounds[0]] += 1.0;
  }

  for (int k = 0; k < distribution.size(); k++) {
    distribution[k] *= norm / n;
  }
}

double calculateMeanSquaredError(const vector<double> &predicted,
                                 const vector<double> &expected) {
  assert(predicted.size() == expected.size());
  const int n = predicted.size();

  double result = 0.0;
  for (int k = 0; k < n; k++) {
    result += pow(predicted[k] - expected[k], 2);
  }

  return result / n;
}
} // namespace

TEST_CASE("Poisson bad arguments", "[poisson]") {
  const int n = 100;
  const double lambda = 0.5;
  const auto method = VSL_RNG_METHOD_POISSON_PTPE;
  int buffer[n];

  VSLStreamStatePtr stream;
  int errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_MCG59, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode = viRngPoisson(0xffff, stream, n, buffer, lambda);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream)") {
    errcode = viRngPoisson(method, nullptr, n, buffer, lambda);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (size)") {
    errcode = viRngPoisson(method, stream, -1, buffer, lambda);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (buffer)") {
    errcode = viRngPoisson(method, stream, n, nullptr, lambda);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fifth parameter (lambda)") {
    const double lambda_ = GENERATE(-123.456, -1.0, 0.0);
    errcode = viRngPoisson(method, stream, n, buffer, lambda_);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Poisson outputs only in the correct range", "[poisson]") {
  const int n = 100'000;
  const double lambda = GENERATE(0.001, 0.1, 1, 10, 27, 100, 500, 1000);
  const uint32_t seed = GENERATE(0, 1, 0x123);
  CAPTURE(n, lambda, seed);

  const auto brngId = VSL_BRNG_MCG59;
  CAPTURE(brngToString(brngId));

  const auto method = VSL_RNG_METHOD_POISSON_PTPE;
  CAPTURE(method);

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> buffer(n);
  errcode = viRngPoisson(method, stream, n, buffer.data(), lambda);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < n; i++) {
    REQUIRE(0 <= buffer[i]);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Poisson output has approximately expected mean", "[poisson]") {
  const int n = 100'000;
  const double lambda = GENERATE(0.001, 0.1, 1, 10, 27, 100, 500, 1000);
  const uint32_t seed = GENERATE(0, 1, 0x123);
  const double predictedMean = lambda;
  const double allowedRelativeError = 0.10; // 10% error
  CAPTURE(n, lambda, seed, allowedRelativeError);

  const auto brngId = VSL_BRNG_MCG59;
  CAPTURE(brngToString(brngId));

  const auto method = VSL_RNG_METHOD_POISSON_PTPE;
  CAPTURE(method);

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> buffer(n);
  errcode = viRngPoisson(method, stream, n, buffer.data(), lambda);
  REQUIRE(errcode == VSL_ERROR_OK);

  const double mean = calculateMean(buffer);

  CAPTURE(mean, predictedMean);
  REQUIRE_THAT(mean, WithinRel(predictedMean, allowedRelativeError));

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Poisson output has approximately expected variance", "[poisson]") {
  const int n = 100'000;
  const double lambda = GENERATE(0.001, 0.1, 1, 10, 27, 100, 500, 1000);
  const int seed = GENERATE(0, 0x123);
  const double predictedVariance = lambda;
  const double allowedRelativeError = 0.11; // 11% error
  CAPTURE(n, lambda, seed, allowedRelativeError);

  const auto brngId = VSL_BRNG_MCG59;
  CAPTURE(brngToString(brngId));

  const auto method = VSL_RNG_METHOD_BINOMIAL_BTPE;
  CAPTURE(method);

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> buffer(n);
  errcode = viRngPoisson(method, stream, n, buffer.data(), lambda);
  REQUIRE(errcode == VSL_ERROR_OK);

  const double variance = calculateVariance(buffer);

  CAPTURE(variance, predictedVariance);
  REQUIRE_THAT(predictedVariance, WithinRel(variance, allowedRelativeError));

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Poisson ICDF output comparable to refng", "[poisson]") {
  const int n = 10'000;
  const double lambda = GENERATE(0.001, 0.01, 0.1, 1, 10, 26.9);
  const int seed = GENERATE(0, 0x123);
  const double maxFailuresFraction = 0.05; // 5%
  CAPTURE(n, lambda, seed, maxFailuresFraction);

  const auto brngId = VSL_BRNG_MCG59;
  CAPTURE(brngToString(brngId));

  const auto method = VSL_RNG_METHOD_POISSON_PTPE;
  CAPTURE(method);

  VSLStreamStatePtr vstream, rstream;

  int errcode = vslNewStream(&vstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> vbuffer(n);
  errcode = viRngPoisson(method, vstream, n, vbuffer.data(), lambda);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> rbuffer(n);
  errcode = riRngPoisson(method, rstream, n, rbuffer.data(), lambda);
  REQUIRE(errcode == VSL_ERROR_OK);

  int failures = 0;
  for (int i = 0; i < n; i++) {
    failures += vbuffer[i] == rbuffer[i] ? 0 : 1;
  }

  const double failuresFraction = (double)failures / n;
  CAPTURE(failures, failuresFraction);
  REQUIRE(failuresFraction <= maxFailuresFraction);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Poisson passes mean squared error test", "[poisson]") {
  const int n = 10'000;
  const double lambda = GENERATE(0.001, 0.1, 1, 10, 27, 100, 500, 1000);
  const double allowedMeanSquaredError = 1e-3;
  CAPTURE(n, lambda);

  const auto brngId = VSL_BRNG_MCG59;
  CAPTURE(brngToString(brngId));

  const auto method = VSL_RNG_METHOD_POISSON_PTPE;
  CAPTURE(method);

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> buffer(n);
  errcode = viRngPoisson(method, stream, n, buffer.data(), lambda);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> bounds(2);
  bounds[0] = *min_element(buffer.begin(), buffer.end());
  bounds[1] = *max_element(buffer.begin(), buffer.end());

  vector<double> expected;
  const double norm = calculateTruncatedPMF(lambda, bounds, expected);
  CAPTURE(bounds[0], bounds[1], norm);

  vector<double> predicted(bounds[1] - bounds[0] + 1);
  transformToDistribution(buffer, bounds, norm, predicted);

  const double meanSquaredError =
      calculateMeanSquaredError(predicted, expected);

  CAPTURE(meanSquaredError, allowedMeanSquaredError);
  REQUIRE(meanSquaredError < allowedMeanSquaredError);

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Poisson expected data", "[poisson][openrng_only]") {
  for (auto [key, reference] : PoissonGolden::reference) {
    const auto [brngId, method, seed, lambda] = key;
    const auto n = reference.size();
    CAPTURE(n, brngId, method, seed, lambda);

    VSLStreamStatePtr rstream;
    int errcode = refNewStream(&rstream, brngId, seed);
    REQUIRE(errcode == VSL_ERROR_OK);

    vector<int> buffer(n);
    errcode = riRngPoisson(method, rstream, n, buffer.data(), lambda);
    REQUIRE(errcode == VSL_ERROR_OK);

    REQUIRE(buffer == reference);

    errcode = refDeleteStream(&rstream);
    REQUIRE(errcode == VSL_ERROR_OK);
  }
}
