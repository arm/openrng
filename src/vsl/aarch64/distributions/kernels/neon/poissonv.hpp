/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <array>
#include <distributions/kernels/poissonv.hpp>
#include <math/log.hpp>

namespace openrng::distribution::poissonv {

/**
 * Approximate Poisson CDF as a Gaussian CDF for inverse transform.
 */
template <> struct poisnormv<CPUFeatures::neon> {
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

    gaussian::ICDF<CPUFeatures::neon>::fill(generator, N, floatView, mean,
                                            sigma);

    for (int64_t i = 0; i < N; i++) {
      buffer[i] = std::ceil(std::max(0.0F, floatView[i]));
    }
  }
};

/**
 * Implements the inverse transformation.
 */
template <> struct icdf<CPUFeatures::neon> {
  void run(Generator *generator, const int64_t N, int32_t *buffer,
           const float lambda) {
    static_assert(sizeof(*buffer) == sizeof(float));
    auto *floatView = reinterpret_cast<float *>(buffer);
    generator->fill(N, floatView);

    const int mean = lambda;
    const int s = std::ceil(std::sqrt(lambda));

    const int domainSize = mean + 8 * s;
    DynamicArray<float> H = calculateH(lambda, domainSize);

    const int domainSubdivisions = 10 * domainSize;
    DynamicArray<int> A = calculateA(domainSubdivisions, H);

    const int uniformToPoissonFactor = domainSubdivisions - 1;

    for (int64_t i = 0; i < N; i++) {
      buffer[i] = generatePoissonVariate(A.data(), H.data(),
                                         uniformToPoissonFactor, floatView[i]);
    }
  }

  int64_t generatePoissonVariate(const int *A, const float *H,
                                 const int uniformToPoissonFactor, float u) {
    int x = A[(int)(uniformToPoissonFactor * u)];

    while (u >= H[x + 1]) {
      x++;
    }

    return x;
  }

  DynamicArray<float> calculateH(const float lambda, const int domainSize) {
    DynamicArray<float> H(domainSize + 1);

    H[0] = 0;
    H[1] = std::exp(-lambda);
    float lastP = H[1];
    for (int k = 2; k < domainSize; k++) {
      lastP = lastP * (lambda / (k - 1));
      H[k] = H[k - 1] + lastP;
    }
    H[domainSize] = 1.001;

    return H;
  }

  DynamicArray<int> calculateA(const int domainSubdivisions,
                               const DynamicArray<float> &H) {
    DynamicArray<int> A(domainSubdivisions);

    const float s = domainSubdivisions - 1;
    int i = 0;
    for (int j = 0; j < domainSubdivisions; j++) {
      const float x = j / s;
      while (!(H[i] <= x && x <= H[i + 1])) {
        i++;
      }
      A[j] = i;
    }

    return A;
  }
};

template <> struct PoisnormV<CPUFeatures::neon> {
  static int fill(Generator *generator, const int64_t N, int32_t buffer[],
                  const double lambda[]) {
    assert(N > 0);
    assert(generator != nullptr);

    for (int64_t i = 0; i < N; i++) {
      /*
          Use a Gaussian approximation for lambda >= 10.
          For cases where this is not satisfied, we use inverse transform.
      */
      if (lambda[i] >= 10) {
        poisnormv<CPUFeatures::neon>().run(generator, 1, &buffer[i], lambda[i]);
      } else {
        icdf<CPUFeatures::neon>().run(generator, 1, &buffer[i], lambda[i]);
      }
    }
    return VSL_ERROR_OK;
  }
};
} // namespace openrng::distribution::poissonv
