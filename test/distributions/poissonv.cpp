/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
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
#include <utils/golden/poissonv.hpp>
#include <utils/names_map.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

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

vector<double> generate_lambda_values(const int n) {
  vector<double> lambda(n, -1.0);
  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, VSL_BRNG_MT19937, 0x123);

  if (errcode != VSL_ERROR_OK) {
    // We return an unusable lambda which breaks the tests.
    return lambda;
  }
  errcode = vdRngUniform(VSL_RNG_METHOD_UNIFORM_STD, stream, n, lambda.data(),
                         0.03, 20.0);
  if (errcode != VSL_ERROR_OK) {
    // We return an unusable lambda which breaks the tests.
    errcode = vslDeleteStream(&stream);
    return vector<double>(n, -1.0);
  }
  errcode = vslDeleteStream(&stream);
  return lambda;
}

} // namespace

TEST_CASE("PoissonV bad arguments", "[poissonv]") {
  const int n = 3;
  const vector<double> lambda = {0.01, 2.5, 15.6};
  const auto method = VSL_RNG_METHOD_POISSONV_POISNORM;
  int buffer[n];

  VSLStreamStatePtr stream;
  int errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_MCG59, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode = viRngPoissonV(0xffff, stream, n, buffer, lambda.data());
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream)") {
    errcode = viRngPoissonV(method, nullptr, n, buffer, lambda.data());
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (size)") {
    errcode = viRngPoissonV(method, stream, -1, buffer, lambda.data());
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (buffer)") {
    errcode = viRngPoissonV(method, stream, n, nullptr, lambda.data());
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fifth parameter (lambda - nullptr)") {
    errcode = viRngPoissonV(method, stream, n, buffer, nullptr);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("PoissonV only fills n values", "[poissonv]") {
  const int n = 100;
  const auto lambda = generate_lambda_values(n);
  const auto method = VSL_RNG_METHOD_POISSONV_POISNORM;
  const auto brngId = VSL_BRNG_MT19937;
  vector<int> buffer(n * 2, -10.0);
  const uint32_t seed = 0x345;
  CAPTURE(n, method, brngId, seed);

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngPoissonV(method, stream, n, buffer.data(), lambda.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = n; i < 2 * n; i++) {
    REQUIRE(buffer[i] == -10);
  }
  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("PoissonV outputs only in the correct range", "[poissonv]") {
  const int n = 10'000;
  const auto lambda = generate_lambda_values(n);

  const uint32_t seed = GENERATE(0, 1, 0x345);
  const auto method = VSL_RNG_METHOD_POISSONV_POISNORM;
  CAPTURE(n, seed, method);

  const auto brngId = VSL_BRNG_MT19937;
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> buffer(n);
  errcode = viRngPoissonV(method, stream, n, buffer.data(), lambda.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < n; i++) {
    REQUIRE(0 <= buffer[i]);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("PoissonV output has approximately expected mean", "[poissonv]") {
  const int n = 100'000;
  const double lambda_v = GENERATE(0.1, 3.4, 12.4);
  const vector<double> lambda(n, lambda_v);
  const uint32_t seed = GENERATE(0, 1, 0x123);
  const auto method = VSL_RNG_METHOD_POISSONV_POISNORM;
  const double predictedMean = lambda_v;
  const double allowedRelativeError = 0.05; // 5% error
  CAPTURE(n, lambda_v, seed, method, allowedRelativeError);

  const auto brngId = VSL_BRNG_MT19937;
  CAPTURE(method, brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> buffer(n);
  errcode = viRngPoissonV(method, stream, n, buffer.data(), lambda.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  const double mean = calculateMean(buffer);

  CAPTURE(mean, predictedMean);
  REQUIRE_THAT(mean, WithinRel(predictedMean, allowedRelativeError));

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("PoissonV output has approximately expected variance", "[poissonv]") {
  const int n = 100'000;
  const double lambda_v = GENERATE(0.1, 3.4, 12.4);
  const vector<double> lambda(n, lambda_v);
  const int seed = GENERATE(0, 0x123);
  const auto method = VSL_RNG_METHOD_POISSONV_POISNORM;
  const double predictedVariance = lambda_v;

  const double allowedRelativeError = 0.05; // 5% error

  CAPTURE(n, lambda_v, seed, method, allowedRelativeError);

  const auto brngId = VSL_BRNG_MT19937;
  CAPTURE(method, brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> buffer(n);
  errcode = viRngPoissonV(method, stream, n, buffer.data(), lambda.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  const double variance = calculateVariance(buffer);

  CAPTURE(variance, predictedVariance);
  REQUIRE_THAT(predictedVariance, WithinRel(variance, allowedRelativeError));

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("PoissonV POISNORM output comparable to refng for inversion method",
          "[poissonv]") {
  const int n = GENERATE(10'000, 100'000);
  const double lambda_v = GENERATE(0.001, 0.01, 0.1, 1, 9.5);
  const vector<double> lambda(n, lambda_v);
  const int seed = GENERATE(0, 0x123);
  const double maxFailuresFraction = 0.05; // 5%
  CAPTURE(n, lambda_v, seed, maxFailuresFraction);

  const auto brngId = VSL_BRNG_MT19937;
  CAPTURE(brngToString(brngId));

  const auto method = VSL_RNG_METHOD_POISSONV_POISNORM;
  CAPTURE(method);

  VSLStreamStatePtr vstream, rstream;

  int errcode = vslNewStream(&vstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> vbuffer(n);
  errcode = viRngPoissonV(method, vstream, n, vbuffer.data(), lambda.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> rbuffer(n);
  errcode = riRngPoissonV(method, rstream, n, rbuffer.data(), lambda.data());
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

TEST_CASE(
    "PoissonV POISNORM output comparable to refng for normal approximation",
    "[poissonv]") {
  const int n = GENERATE(1'000'000);
  const double lambda_v = GENERATE(11.5, 25.0);
  const vector<double> lambda(n, lambda_v);
  const int seed = GENERATE(0, 0x123);

  /*
  For this test, the number of allowed failures is slightly
  relaxed, due to the Normal approximation
  */
  const double maxFailuresFraction = 0.20; // 20%
  CAPTURE(n, lambda_v, seed, maxFailuresFraction);

  const auto brngId = VSL_BRNG_MT19937;
  CAPTURE(brngToString(brngId));

  const auto method = VSL_RNG_METHOD_POISSONV_POISNORM;
  CAPTURE(method);

  VSLStreamStatePtr vstream, rstream;

  int errcode = vslNewStream(&vstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> vbuffer(n);
  errcode = viRngPoissonV(method, vstream, n, vbuffer.data(), lambda.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> rbuffer(n);
  errcode = riRngPoissonV(method, rstream, n, rbuffer.data(), lambda.data());
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

TEST_CASE("PoissonV passes mean squared error test", "[poissonv]") {
  const int n = 10'000;
  const double lambda_v = GENERATE(0.1, 100);
  const vector<double> lambda(n, lambda_v);
  const auto method = VSL_RNG_METHOD_POISSONV_POISNORM;
  const double allowedMeanSquaredError = 1e-3;
  CAPTURE(n, lambda_v, method);

  const auto brngId = VSL_BRNG_MCG59;
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> buffer(n);
  errcode = viRngPoissonV(method, stream, n, buffer.data(), lambda.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> bounds(2);
  bounds[0] = *min_element(buffer.begin(), buffer.end());
  bounds[1] = *max_element(buffer.begin(), buffer.end());

  vector<double> expected;
  const double norm = calculateTruncatedPMF(lambda_v, bounds, expected);
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

TEST_CASE("Poisson and PoissonV consistency", "[poissonv]") {
  const int n = 100'000;
  const double tolerance = 0.025;
  const double lambda_v = GENERATE(0.01, 10.0);
  const vector<double> lambda(n, lambda_v);
  const auto methodV = VSL_RNG_METHOD_POISSONV_POISNORM;
  CAPTURE(lambda_v);

  const auto brngId = VSL_BRNG_MT19937;
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr streamPV;
  VSLStreamStatePtr streamP;
  int errcode = vslNewStream(&streamPV, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&streamP, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> bufferPV(n);
  errcode = viRngPoissonV(methodV, streamPV, n, bufferPV.data(), lambda.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> bufferP(n);
  errcode = viRngPoisson(methodV, streamP, n, bufferP.data(), lambda_v);
  REQUIRE(errcode == VSL_ERROR_OK);

  int diffs = 0;
  for (int i = 0; i < n; i++) {
    if (bufferPV[i] != bufferP[i]) {
      diffs++;
    }
  }

  REQUIRE((double)diffs / n < tolerance);

  errcode = vslDeleteStream(&streamP);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&streamPV);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("PoissonV expected data", "[poissonv][openrng_only]") {
  for (auto [key, reference] : PoissonVGolden::reference) {
    const auto [brngId, method, seed, lambda] = key;
    const auto n = reference.size();
    CAPTURE(n, brngId, method, seed);

    VSLStreamStatePtr rstream;
    int errcode = refNewStream(&rstream, brngId, seed);
    REQUIRE(errcode == VSL_ERROR_OK);

    vector<int> buffer(n);
    errcode = riRngPoissonV(method, rstream, n, buffer.data(), lambda.data());
    REQUIRE(errcode == VSL_ERROR_OK);

    REQUIRE(buffer == reference);

    errcode = refDeleteStream(&rstream);
    REQUIRE(errcode == VSL_ERROR_OK);
  }
}