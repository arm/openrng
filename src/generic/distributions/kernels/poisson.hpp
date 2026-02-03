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

namespace openrng::distribution::poisson {
template <CPUFeatures> struct poisnorm {};

/**
 * Approximate Poisson CDF as a Gaussian CDF for inverse transform.
 */
template <> struct poisnorm<CPUFeatures::generic> {
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

    for (int i = 0; i < N; i++) {
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

    for (int i = 0; i < N; i++) {
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

template <CPUFeatures> struct ptpe {};

/**
 * PTPE (Poisson, Triangle, Parallelogram, Exponential)
 * generates binomial elements using accept/reject approach.
 */
template <> struct ptpe<CPUFeatures::generic> {
  Generator *generator;
  double mu;
  int M;
  double xL;
  double xR;
  double lL;
  double lR;
  double a;
  double c;
  double p1;
  double p2;
  double p3;
  double p4;

  void run(Generator *generator_, const int64_t N, int32_t *buffer,
           const double lambda) {
    generator = generator_;
    mu = lambda;
    M = std::floor(mu);
    p1 = std::floor(2.195 * std::sqrt(M) - 2.2) + 0.5;
    c = 0.133 + 8.56 / (6.8 + mu);
    xL = M + 0.5 - p1;
    xR = M + 0.5 + p1;
    a = (mu - xL) / mu;
    lL = a * (1 + a / 2);
    a = (xR - mu) / xR;
    lR = a * (1 + a / 2);
    p2 = p1 * (1 + 2 * c);
    p3 = p2 + (0.109 + 8.25 / (10.86 + mu)) / lL;
    p4 = p3 + c / lR;

    for (int i = 0; i < N; i++) {
      buffer[i] = step1();
    }
  }

  // Generate new variates and check triangle region
  inline int64_t step1() {
    double uv[2] = {};
    generator->fill(2, uv);
    const double u = uv[0] * p4;
    const double v = uv[1];
    if (u > p1)
      return step2(u, v);
    const int64_t y = M + 0.5 - p1 * v + u;
    return y;
  }

  // Check parallelogram region
  inline int64_t step2(const double u, double v) {
    if (u > p2)
      return step3(u, v);
    const double x = xL + (u - p1) / c;
    v = v * c + 1.0 - std::abs(M - x + 0.5) / p1;
    if (v > 1.0)
      return step1();
    const int64_t y = x;
    return step5_0(y, v);
  }

  // Check left exponential tail
  inline int64_t step3(const double u, double v) {
    if (u > p3)
      return step4(u, v);
    const int64_t y = xL + std::log(v) / lL;
    if (y < 0)
      return step1();
    v = v * lL * (u - p2);
    return step5_0(y, v);
  }

  // Check right exponential tail
  inline int64_t step4(const double u, double v) {
    const int64_t y = xR - std::log(v) / lR;
    v = v * lR * (u - p3);
    return step5_0(y, v);
  }

  // Determine appropriate evaluation method
  inline int64_t step5_0(const int64_t y, const double v) {
    if (M >= 100 && y > 50)
      return step5_2(y, v);
    return step5_1(y, v);
  }

  // Recursive evaluation
  inline int64_t step5_1(const int64_t y, const double v) {
    double F = 1.0;
    if (M < y) {
      for (int j = M + 1; j <= y; j++) {
        F = F * mu / j;
      }
    } else if (M > y) {
      for (int j = y + 1; j <= M; j++) {
        F = F * j / mu;
      }
    }
    if (v > F)
      return step1();
    return y;
  }

  // Squeezing
  inline int64_t step5_2(const int64_t y, const double v) {
    const double q = (mu - y) / y;
    const double UB =
        y - mu + (y + 0.5) * q * (1 + q * (-0.5 + q / 3)) + 0.00084;
    const double A = std::log(v);
    if (A > UB)
      return step1();
    double D = (y + 0.5) * std::pow(q, 4) / 4;
    if (q < 0)
      D = D / (1 + q);
    if (A < (UB - D - 0.004))
      return y;
    return step5_3(y, A);
  }

  // Accept/reject test
  inline int64_t step5_3(const int64_t y, const double A) {
    double lnF = (M + 0.5) * std::log(M / mu);
    lnF += (y + 0.5) * std::log(mu / y - M + y);
    lnF += (1.0 / M - 1.0 / y) / 12.0;
    lnF += (1.0 / std::pow(y, 3.0) - 1.0 / std::pow(M, 3.0)) / 360.0;
    if (A > lnF) {
      return step1();
    }
    return y;
  }
};

template <CPUFeatures> struct PTPE {};
template <> struct PTPE<CPUFeatures::generic> {
  static int fill(Generator *generator, const int64_t N, int32_t buffer[],
                  const double lambda) {
    assert(N > 0);
    assert(generator != nullptr);
    /*
      PTPE produces valid variates for lambda >= 27.
      For cases where this is not satisfied, we use inverse transform.
    */

    if (lambda >= 27) {
      ptpe<CPUFeatures::generic>().run(generator, N, buffer, lambda);
    } else {
      icdf<CPUFeatures::generic>().run(generator, N, buffer, lambda);
    }
    return VSL_ERROR_OK;
  }
};

template <CPUFeatures> struct Poisnorm {};
template <> struct Poisnorm<CPUFeatures::generic> {
  static int fill(Generator *generator, const int64_t N, int32_t buffer[],
                  const double lambda) {
    assert(N > 0);
    assert(generator != nullptr);
    /*
      Use a Gaussian approximation for lambda >= 10.
      For cases where this is not satisfied, we use inverse transform.
    */
    if (lambda >= 10) {
      poisnorm<CPUFeatures::generic>().run(generator, N, buffer, lambda);
    } else {
      icdf<CPUFeatures::generic>().run(generator, N, buffer, lambda);
    }
    return VSL_ERROR_OK;
  }
};

} // namespace openrng::distribution::poisson
