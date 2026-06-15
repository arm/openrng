/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once
#include <cassert>
#include <cmath>
#include <distributions/kernels/gaussian_icdf.hpp>
#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/concepts.hpp>
#include <utils/cpu_features.hpp>
#include <utils/dynamic_array.hpp>

namespace openrng::distribution::poissonv {
template <CPUFeatures> struct poisnormv {};

/**
 * Approximate Poisson CDF as a Gaussian CDF for inverse transform.
 */
template <> struct poisnormv<CPUFeatures::generic> {
  void run(Generator *generator, const int64_t N, int32_t *buffer,
           const double lambda) {
    static_assert(sizeof(*buffer) == sizeof(float));
    auto *floatView = reinterpret_cast<float *>(buffer);

    /*
      We subtract 0.5 from lambda when calculating the mean to achieve
      a result equivalent to continuity correction:
        P(X <= k) ~ P(Y <= k + 0.5),
      where X and Y are Poisson and Gaussian distributions, respectively.
      This reduces error size by about 4 times compared to the naive
      approximation:
        P(X <= k) ~ P(Y <= k).
    */
    const float mean = lambda - 0.5;
    const float sigma = std::sqrt(lambda);

    gaussian::ICDF<CPUFeatures::generic>::fill(generator, N, floatView, mean,
                                               sigma);

    for (int64_t i = 0; i < N; i++) {
      buffer[i] = std::ceil(std::max(0.0F, floatView[i]));
    }
  }
};

template <CPUFeatures> struct icdf {};

/**
 * Implements the inverse transformation.
 */
template <> struct icdf<CPUFeatures::generic> {
  void run(Generator *generator, const int64_t N, int32_t *buffer,
           const double lambda) {
    static_assert(sizeof(*buffer) == sizeof(float));
    auto *floatView = reinterpret_cast<float *>(buffer);
    generator->fill(N, floatView);

    const double p = std::exp(-lambda);

    for (int64_t i = 0; i < N; i++) {
      buffer[i] = generatePoissonVariate(floatView[i], lambda, p);
    }
  }

  inline int64_t generatePoissonVariate(double u, const double lambda,
                                        double p) {
    int k = 0;
    const double uMax = 1.0 - std::pow(2.0, -31.0);
    u = std::min(u, uMax);
    double s = p;

    while (u > s) {
      k++;
      p = p * lambda / k;
      s = s + p;
    }

    return k;
  }
};

template <CPUFeatures> struct PoisnormV {};
template <> struct PoisnormV<CPUFeatures::generic> {
  static int fill(Generator *generator, const int64_t N, int32_t buffer[],
                  const double lambda[]) {
    assert(N > 0);
    assert(generator != nullptr);
    /*
      Use a Gaussian approximation for lambda_i >= 10.
      For cases where this is not satisfied, we use inverse transform.
    */
    for (int64_t i = 0; i < N; i++) {
      if (lambda[i] >= 10) {
        poisnormv<CPUFeatures::generic>().run(generator, 1, &buffer[i],
                                              lambda[i]);
      } else {
        icdf<CPUFeatures::generic>().run(generator, 1, &buffer[i], lambda[i]);
      }
    }
    return VSL_ERROR_OK;
  }
};

} // namespace openrng::distribution::poissonv
