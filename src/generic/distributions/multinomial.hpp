/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/multinomial.hpp>
#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/choose_distribution_kernel.hpp>
#include <utils/concepts.hpp>

#include <cmath>
#include <numeric>

namespace openrng::distribution::multinomial {

int fill(const int64_t method, Generator *generator, const int64_t n,
         int32_t r[], const int32_t ntrial, const int32_t k, const double p[]) {
  if (n < 0 || generator == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (r == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (n == 0) {
    return VSL_ERROR_OK;
  }

  if (ntrial < 0) {
    return VSL_ERROR_BADARGS;
  }

  if (k <= 0) {
    return VSL_ERROR_BADARGS;
  }

  if (method != VSL_RNG_METHOD_MULTINOMIAL_MULTPOISSON) {
    return VSL_ERROR_BADARGS;
  }

  /*
   Other VSL libraries return an error if the sum of the groups probabilities
   has an discrepancy >= 1.0e-3 with respect to 1. Here, we adopt the same
   tolerance. Also, we enforce that each probability must be greater or equal
   to 0.
   */

  if (p[0] < 0) {
    return VSL_DISTR_MULTINOMIAL_BAD_PROBABILITY_ARRAY;
  }
  DynamicArray<double> cumulative_prob(k);
  cumulative_prob[0] = p[0];
  for (int groupId = 1; groupId < k; groupId++) {
    if (p[groupId] < 0) {
      return VSL_DISTR_MULTINOMIAL_BAD_PROBABILITY_ARRAY;
    }
    cumulative_prob[groupId] = cumulative_prob[groupId - 1] + p[groupId];
  }

  if (fabs(cumulative_prob[k - 1] - 1.0) > 1.0e-3) {
    return VSL_DISTR_MULTINOMIAL_BAD_PROBABILITY_ARRAY;
  }

  assert(cumulative_prob[k - 2] <= 1.0);
  cumulative_prob[k - 1] = 1.0;

  return choose<Multinomial>(generator, n, r, ntrial, k, p,
                             cumulative_prob.data());
}

} // namespace openrng::distribution::multinomial
