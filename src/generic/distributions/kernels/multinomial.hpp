/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once
#include <cassert>
#include <cmath>
#include <distributions/kernels/poisson.hpp>
#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/concepts.hpp>
#include <utils/cpu_features.hpp>
#include <utils/dynamic_array.hpp>

namespace openrng::distribution::multinomial {

template <CPUFeatures> struct Multinomial {};
template <> struct Multinomial<CPUFeatures::generic> {
  static int fill(Generator *generator, const int64_t N, int32_t buffer[],
                  const int32_t ntrial, const int32_t k, const double p[],
                  const double cumulative_prob[]) {
    assert(N > 0);
    assert(generator != nullptr);

    /*
     * If ntrial == 0, then the only admissible configuration is that all
     * elements of the buffer are set to zero.
     */
    if (ntrial == 0) {
      for (int i = 0; i < N * k; i++) {
        buffer[i] = 0;
      }
      return VSL_ERROR_OK;
    }

    /*
      The parameter lambda_start scales each probability
      of category so that the Poisson values generated can be
      around the proportion of ntrial for that category. Otherwise
      each Poisson with probability p_i, would generate mostly
      values around 0 or 1, being 0< p_i < 1. This approach
      has been taken from M.B. Brown and J. Bromberg, "An efficient two-stage
      procedure for generating random variates from the multinomial
      distribution", The American Statistician, 38, 1984, 216-219.
    */
    double lambda_start = 1.0;
    const auto ntrial_sq = static_cast<int32_t>(std::round(std::sqrt(ntrial)));
    if (k <= ntrial_sq) {
      lambda_start = ntrial - k;
    } else {
      lambda_start = ntrial - ntrial_sq -
                     static_cast<int32_t>(std::round(std::sqrt(k - ntrial_sq)));
    }

    /*
     * This differs from the lambda_hat approximation of
     * the reference paper. This approximation reduces the
     * percentage error of mean and variance.
     */
    lambda_start = lambda_start / 2;

    for (int i = 0; i < N; i++) {
      fill_element_i(generator, lambda_start, ntrial, k, &buffer[i * k], p,
                     cumulative_prob);
    }

    return VSL_ERROR_OK;
  }

  static inline void fill_element_i(Generator *generator,
                                    const double lambda_start,
                                    const int32_t ntrial, const int32_t k,
                                    int32_t *buffer, const double p[],
                                    const double cumulative_prob[]) {
    int total = 0;
    do {
      total = 0;
      for (int groupId = 0; groupId < k; groupId++) {
        const double poisson_par = lambda_start * p[groupId];
        if (poisson_par > 0) {
          openrng::distribution::poisson::poisnorm<CPUFeatures::generic>().run(
              generator, 1, &buffer[groupId], poisson_par);
        } else {
          buffer[groupId] = 0.0;
        }
      }

      for (int groupId = 0; groupId < k; groupId++) {
        total += buffer[groupId];
      }
    } while (total > ntrial);

    if (total < ntrial) {
      DynamicArray<double> uniforms(ntrial - total);
      generator->fill(ntrial - total, uniforms.data());

      for (int j = 0; j < ntrial - total; j++) {
        for (int groupId = 0; groupId < k; groupId++) {
          if (uniforms[j] <= cumulative_prob[groupId]) {
            buffer[groupId] += 1;
            break;
          }
        }
      }
    }
  }
};

} // namespace openrng::distribution::multinomial
