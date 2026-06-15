/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <openrng.h>
#include <refng_vsl.h>
#include <utils/golden/multinomial.hpp>
#include <utils/names_map.hpp>
#include <vector>

using std::vector;

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("Multinomial bad arguments", "[multinomial]") {
  const int n = 1;
  const double p[3] = {1.0, 2.0, 3.0};
  const int ntrial = 1;
  const int k = 3;
  const auto method = VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON;
  const auto brngId = VSL_BRNG_MT19937;

  VSLStreamStatePtr stream;
  int buffer[1];

  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode = viRngMultinomial(0xffff, stream, n, buffer, ntrial, k, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream)") {
    errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, nullptr,
                               n, buffer, ntrial, k, p);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (size)") {
    errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, stream,
                               -1, buffer, ntrial, k, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (buffer)") {
    errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, stream,
                               n, nullptr, ntrial, k, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fifth parameter (ntrial)") {
    errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, stream,
                               n, buffer, -1, k, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the sixth parameter (k)") {
    errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, stream,
                               n, buffer, ntrial, 0, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the seventh parameter (p) - nullptr") {
    errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, stream,
                               n, buffer, ntrial, k, nullptr);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the seventh parameter (p) - Wrong Sum") {
    errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, stream,
                               n, buffer, ntrial, k, p);
    REQUIRE(errcode == VSL_DISTR_MULTINOMIAL_BAD_PROBABILITY_ARRAY);
  }

  SECTION("Testing the seventh parameter (p) - Negative values") {
    const double negative_p[3] = {0.5, -1.0, 0.5};
    errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, stream,
                               n, buffer, ntrial, k, negative_p);
    REQUIRE(errcode == VSL_DISTR_MULTINOMIAL_BAD_PROBABILITY_ARRAY);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Multinomial returns all zeros for ntrial==0 regardless of k",
          "[multinomial]") {
  const int n = GENERATE(1, 10, 100);
  const int ntrial = 0;
  const int k = GENERATE(2, 6, 10);
  const auto method = VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON;
  const auto brngId = VSL_BRNG_MT19937;

  CAPTURE(n, ntrial);

  std::vector<double> p(k);
  for (int el = 0; el < k; el++) {
    p[el] = 1.0 / (double)k;
  }

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<int> buffer(n * k, -1);
  errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, stream, n,
                             buffer.data(), ntrial, k, p.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int el = 0; el < n * k; el++) {
    REQUIRE(buffer[el] == 0);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Multinomial returns zeros for any category with probability zero",
          "[multinomial]") {
  const int n = GENERATE(1, 10, 100, 1000, 10000);
  const int ntrial = 50;
  const int k = GENERATE(15, 25, 30);
  const std::vector<int> zero_prob_indexes =
      GENERATE(std::vector<int>{1, 3, 6}, std::vector<int>{2, 4, 6, 10},
               std::vector<int>{3, 5, 7, 9, 11, 13});
  const auto method = VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON;
  const auto brngId = VSL_BRNG_MT19937;

  CAPTURE(n, ntrial, zero_prob_indexes);
  REQUIRE(k - zero_prob_indexes.size() > 0);
  /*
    We generate a vector of probabilities with zero
    probability at the zero_p_indexes. At the other
    indexes we set the constant value of 1 / (k - zero_p_indexes.size() )
  */
  std::vector<double> p(k);
  for (int el = 0; el < k; el++) {
    const bool must_set_to_zero =
        std::find(zero_prob_indexes.begin(), zero_prob_indexes.end(), el) !=
        zero_prob_indexes.end();
    if (must_set_to_zero) {
      p[el] = 0.0;
    } else {
      p[el] = 1.0 / (k - zero_prob_indexes.size());
    }
  }

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<int> buffer(n * k, -1);
  errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, stream, n,
                             buffer.data(), ntrial, k, p.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int observation = 0; observation < n; observation++) {
    int total = 0;
    for (int groupId = 0; groupId < k; groupId++) {
      total += buffer[observation * k + groupId];
    }
    REQUIRE(total == ntrial);

    for (const auto zero_index : zero_prob_indexes) {
      REQUIRE(buffer[observation * k + zero_index] == 0);
    }
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Multinomial values sum to N", "[multinomial]") {
  const int n = GENERATE(5, 10, 100);
  const int ntrial = GENERATE(2, 4, 10);
  const int k = 4;
  const auto method = VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON;
  const auto brngId = VSL_BRNG_MT19937;

  std::vector<double> p(k);
  for (int i = 0; i < k; i++) {
    p[i] = 1.0 / (double)k;
  }

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<int> buffer(n * k);
  errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, stream, n,
                             buffer.data(), ntrial, k, p.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int r = 0; r < n; r++) {
    int32_t total = 0;
    for (int el = 0; el < k; el++) {
      total += buffer[r * k + el];
    }
    CAPTURE(r);
    REQUIRE(total == ntrial);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Multinomial with 1 family returns ntrial for each realization",
          "[multinomial]") {
  const int n = GENERATE(5, 10, 100);
  const int ntrial = GENERATE(2, 4, 10);
  const int k = 1;
  const auto method = VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON;
  const auto brngId = VSL_BRNG_MT19937;

  std::vector<double> p = {1.0};

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<int> buffer(n * k);
  errcode =
      viRngMultinomial(method, stream, n, buffer.data(), ntrial, k, p.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int r = 0; r < n; r++) {
    CAPTURE(r);
    REQUIRE(buffer[r] == ntrial);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Multinomial returns the same values to refng",
          "[multinomial][openrng_only]") {
  const int n = GENERATE(5, 10, 100, 1000);
  const int ntrial = GENERATE(2, 4, 10);
  const int k = GENERATE(4, 5, 7);
  const auto method = VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON;
  const auto brngId = VSL_BRNG_MT19937;

  std::vector<double> p(k);
  for (int i = 0; i < k; i++) {
    p[i] = 1.0 / (double)k;
  }

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  VSLStreamStatePtr rstream;
  errcode = refNewStream(&rstream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<int> buffer(n * k);
  errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, stream, n,
                             buffer.data(), ntrial, k, p.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<int> rbuffer(n * k);
  errcode = riRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, rstream, n,
                             rbuffer.data(), ntrial, k, p.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int r = 0; r < n; r++) {
    for (int el = 0; el < k; el++) {
      CAPTURE(r, n, ntrial, k, el);
      REQUIRE(buffer[r * k + el] == rbuffer[r * k + el]);
    }
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Multinomial has expected mean", "[multinomial]") {
  const int n = 50000;
  const int ntrial = GENERATE(10, 25, 40);
  const int k = GENERATE(4, 5, 7);
  const auto method = VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON;
  const auto brngId = VSL_BRNG_MT19937;

  const double max_allowed_error_mean = 0.01;

  std::vector<double> p(k);
  for (int i = 0; i < k; i++) {
    p[i] = 1.0 / (double)k;
  }

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<int> buffer(n * k);
  errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, stream, n,
                             buffer.data(), ntrial, k, p.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  // Each category of the multinomial has the mean
  // of a binomial distribution.
  const double reference_mean = ntrial * p[0];

  for (int el = 0; el < k; el++) {
    int total = 0;
    for (int r = 0; r < n; r++) {
      total += buffer[r * k + el];
    }
    CAPTURE(n, ntrial, k, el);
    double value = (double)total / (double)n;
    REQUIRE_THAT(value, WithinRel(reference_mean, max_allowed_error_mean));
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Multinomial has expected variance", "[multinomial]") {
  const int n = 10000;
  const int ntrial = GENERATE(25, 40);
  const int k = GENERATE(4, 5);
  const auto method = VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON;
  const auto brngId = VSL_BRNG_MT19937;

  const double max_allowed_error_variance = 0.07;

  std::vector<double> p(k);
  for (int i = 0; i < k; i++) {
    p[i] = 1.0 / (double)k;
  }

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<int> buffer(n * k);
  errcode = viRngMultinomial(VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON, stream, n,
                             buffer.data(), ntrial, k, p.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  // Each category of the multinomial has the mean
  // of a binomial distribution.
  const double reference_variance = ntrial * p[0] * (1.0 - p[0]);

  for (int el = 0; el < k; el++) {
    // Calculate sample mean
    int total = 0;
    for (int r = 0; r < n; r++) {
      total += buffer[r * k + el];
    }
    const double mean = (double)total / (double)n;

    double total_variance = 0;
    for (int r = 0; r < n; r++) {
      total_variance += ((double)buffer[r * k + el] - mean) *
                        ((double)buffer[r * k + el] - mean);
    }
    const double value = (double)total_variance / (double)n;

    CAPTURE(n, ntrial, k, el);
    REQUIRE_THAT(value,
                 WithinRel(reference_variance, max_allowed_error_variance));
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Multinomial expected data", "[multinomial][openrng_only]") {
  for (auto [key, reference] : MultinomialGolden::reference) {
    const auto [brngId, method, seed, ntrial, k] = key;
    const int n = reference.size() / k;
    CAPTURE(n, brngId, method, seed, ntrial, k);

    std::vector<double> p(k);
    for (int i = 0; i < k; i++) {
      p[i] = 1.0 / (double)k;
    }

    VSLStreamStatePtr rstream;
    int errcode = refNewStream(&rstream, brngId, seed);
    REQUIRE(errcode == VSL_ERROR_OK);

    vector<int> buffer(n * k);
    errcode = riRngMultinomial(method, rstream, n, buffer.data(), ntrial, k,
                               p.data());
    REQUIRE(errcode == VSL_ERROR_OK);

    REQUIRE(buffer == reference);

    errcode = refDeleteStream(&rstream);
    REQUIRE(errcode == VSL_ERROR_OK);
  }
}
