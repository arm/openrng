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
#include <utils/golden/binomial.hpp>
#include <utils/names_map.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;
using std::min_element, std::max_element;
using std::vector, std::pow;

namespace {

/*
  A more numerically stable calculation of binomial
  probability for arbitrary n and p.
*/
double P(const double p, int n, int k) {
  assert(n >= k);
  assert(k >= 0);

  double q = k * std::log(p) + (n - k) * std::log(1.0 - p);
  while (k > 0) {
    q += std::log((double)n / (double)k);
    n--;
    k--;
  }

  return std::exp(q);
}

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

double calculateTruncatedPMF(const int ntrial, const double p,
                             vector<int> &bounds, vector<double> &PMF) {
  assert(bounds.size() == 2);
  int k = bounds[0];

  double r = P(p, ntrial, bounds[0]);
  for (; k <= bounds[1]; k++) {
    PMF.push_back(r);
    r = r * (p / (1.0 - p)) * ((ntrial - k) / (k + 1.0));
  }

  r = 0.0;
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

TEST_CASE("Binomial bad arguments", "[binomial]") {
  const int n = 100;
  const int ntrial = 20;
  const double p = 0.5;
  const auto method = VSL_RNG_METHOD_BINOMIAL_BTPE;
  int buffer[n];

  VSLStreamStatePtr stream;
  int errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_MCG59, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode = viRngBinomial(0xffff, stream, n, buffer, ntrial, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream)") {
    errcode = viRngBinomial(method, nullptr, n, buffer, ntrial, p);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (size)") {
    errcode = viRngBinomial(method, stream, -1, buffer, ntrial, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (buffer)") {
    errcode = viRngBinomial(method, stream, n, nullptr, ntrial, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fifth parameter (ntrial)") {
    errcode = viRngBinomial(method, stream, n, buffer, -1, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the sixth parameter (p)") {
    const double bad_p = GENERATE_COPY(-0.1, 1.1);
    errcode = viRngBinomial(method, stream, n, buffer, ntrial, bad_p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

/**
 * The binomial distribution is supported on the set {0,1,2,...,ntrial}.
 */
TEST_CASE("Binomial outputs only in the correct range", "[binomial]") {
  const int n = 100'000;
  const int ntrial = GENERATE_COPY(10, 100, 1'000, 10'000);
  const double p = GENERATE_COPY(0.0001, 0.01, 0.1, 0.25, 0.5);
  const double seed = GENERATE_COPY(0, 1, 0x123);
  CAPTURE(n, ntrial, seed, p);

  const auto brngId = VSL_BRNG_MCG59;
  CAPTURE(brngToString(brngId));

  const auto method = VSL_RNG_METHOD_BINOMIAL_BTPE;
  CAPTURE(method);

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> buffer(n);
  errcode = viRngBinomial(method, stream, n, buffer.data(), ntrial, p);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < n; i++) {
    REQUIRE((0 <= buffer[i] && buffer[i] <= ntrial));
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Binomial output has approximately expected mean", "[binomial]") {
  const int n = 100'000;
  const int ntrial = GENERATE_COPY(10, 100, 1'000, 10'000);
  const double p = GENERATE_COPY(0.0001, 0.01, 0.1, 0.25, 0.5);
  const double seed = GENERATE_COPY(0, 1, 0x123);
  const double predictedMean = ntrial * p;
  const double allowedRelativeError = 0.10; // 10% error
  CAPTURE(n, ntrial, p, allowedRelativeError);

  const auto brngId = VSL_BRNG_MCG59;
  CAPTURE(brngToString(brngId));

  const auto method = VSL_RNG_METHOD_BINOMIAL_BTPE;
  CAPTURE(method);

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> buffer(n);
  errcode = viRngBinomial(method, stream, n, buffer.data(), ntrial, p);
  REQUIRE(errcode == VSL_ERROR_OK);

  const double mean = calculateMean(buffer);

  CAPTURE(mean, predictedMean);
  REQUIRE_THAT(mean, WithinRel(predictedMean, allowedRelativeError));

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Binomial output has approximately expected variance", "[binomial]") {
  const int n = 100'000;
  const int ntrial = GENERATE_COPY(10, 100, 1'000, 10'000);
  const double p = GENERATE_COPY(0.01, 0.1, 0.25, 0.5);
  const double predictedVariance = ntrial * p * (1. - p);
  const double allowedRelativeError = 0.10; // 10% error
  CAPTURE(n, ntrial, p, allowedRelativeError);

  const auto brngId = VSL_BRNG_MCG59;
  CAPTURE(brngToString(brngId));

  const auto method = VSL_RNG_METHOD_BINOMIAL_BTPE;
  CAPTURE(method);

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> buffer(n);
  errcode = viRngBinomial(method, stream, n, buffer.data(), ntrial, p);
  REQUIRE(errcode == VSL_ERROR_OK);

  const double variance = calculateVariance(buffer);

  CAPTURE(variance, predictedVariance);
  REQUIRE_THAT(predictedVariance, WithinRel(variance, allowedRelativeError));

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Binomial ICDF output comparable to refng", "[binomial]") {
  const int n = 10'000;
  const int ntrial = GENERATE_COPY(100, 1'000, 10'000);

  // n * min(1-p,p) <= 30, ICDF is being used
  const double p = GENERATE_COPY(0.1, 1.0, 5.0, 29.0) / ntrial;
  const int seed = GENERATE_COPY(0, 0x123);
  const double maxFailuresFraction = 0.05; // 5%
  CAPTURE(n, ntrial, p, seed, maxFailuresFraction);

  const auto brngId = VSL_BRNG_MCG59;
  CAPTURE(brngToString(brngId));

  const auto method = VSL_RNG_METHOD_BINOMIAL_BTPE;
  CAPTURE(method);

  VSLStreamStatePtr vstream, rstream;

  int errcode = vslNewStream(&vstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> vbuffer(n);
  errcode = viRngBinomial(method, vstream, n, vbuffer.data(), ntrial, p);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> rbuffer(n);
  errcode = riRngBinomial(method, rstream, n, rbuffer.data(), ntrial, p);
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

TEST_CASE("Binomial passes mean squared error test", "[binomial]") {
  const int n = 10'000;
  const int ntrial = GENERATE_COPY(10, 100, 1'000, 10'000);
  const double p = GENERATE_COPY(0.0001, 0.01, 0.1, 0.25, 0.5);
  const double allowedMeanSquaredError = 1e-3;
  CAPTURE(n, ntrial, p);

  const auto brngId = VSL_BRNG_MCG59;
  CAPTURE(brngToString(brngId));

  const auto method = VSL_RNG_METHOD_BINOMIAL_BTPE;
  CAPTURE(method);

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> buffer(n);
  errcode = viRngBinomial(method, stream, n, buffer.data(), ntrial, p);
  REQUIRE(errcode == VSL_ERROR_OK);

  vector<int> bounds(2);
  bounds[0] = *min_element(buffer.begin(), buffer.end());
  bounds[1] = *max_element(buffer.begin(), buffer.end());

  vector<double> expected;
  const double norm = calculateTruncatedPMF(ntrial, p, bounds, expected);
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

TEST_CASE("Binomial expected data", "[binomial][openrng_only]") {
  for (auto [key, reference] : BinomialGolden::reference) {
    const auto [brngId, method, seed, ntrial, p] = key;
    const auto n = reference.size();
    CAPTURE(n, brngId, method, seed, ntrial, p);

    VSLStreamStatePtr rstream;
    int errcode = refNewStream(&rstream, brngId, seed);
    REQUIRE(errcode == VSL_ERROR_OK);

    vector<int> buffer(n);
    errcode = riRngBinomial(method, rstream, n, buffer.data(), ntrial, p);
    REQUIRE(errcode == VSL_ERROR_OK);

    REQUIRE(buffer == reference);

    errcode = refDeleteStream(&rstream);
    REQUIRE(errcode == VSL_ERROR_OK);
  }
}
