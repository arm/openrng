/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <array>
#include <distributions/kernels/poisson.hpp>
#include <math/log.hpp>

namespace openrng::distribution::poisson {

/**
 * Approximate Poisson CDF as a Gaussian CDF for inverse transform.
 */
template <> struct poisnorm<CPUFeatures::neon> {
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

    for (int i = 0; i < N; i++) {
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

    for (int i = 0; i < N; i++) {
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

/**
 * PTPE (Poisson, Triangle, Parallelogram, Exponential)
 * generates binomial elements using accept/reject approach.
 */
template <> struct ptpe<CPUFeatures::neon> {
  Generator *generator;
  float mu;
  int M;
  float xL;
  float xR;
  float lL;
  float lR;
  float a;
  float c;
  float p1;
  float p2;
  float p3;
  float p4;

  static const int nVariates = 100;
  std::array<float, nVariates> inputVariates{};
  int jInput;

  void run(Generator *generator_, const int64_t N, int32_t *buffer,
           const float lambda) {
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
    jInput = nVariates;

    for (int i = 0; i < N; i++) {
      buffer[i] = step1();
    }
  }

  void refreshInputVariates() {
    if (jInput == nVariates) {
      generator->fill(nVariates, inputVariates.data());
      jInput = 0;
    }
  }

  // Generate new variates and check triangle region
  int64_t step1() {
    refreshInputVariates();
    const float u = inputVariates[jInput] * p4;
    const float v = inputVariates[jInput + 1];
    jInput += 2;
    if (u > p1)
      return step2(u, v);
    const int64_t y = M + 0.5 - p1 * v + u;
    return y;
  }

  // Check parallelogram region
  int64_t step2(const float u, float v) {
    if (u > p2)
      return step3(u, v);
    const float x = xL + (u - p1) / c;
    v = v * c + 1.0 - std::abs(M - x + 0.5) / p1;
    if (v > 1.0)
      return step1();
    const int64_t y = x;
    return step5_0(y, v);
  }

  // Check left exponential tail
  int64_t step3(const float u, float v) {
    if (u > p3)
      return step4(u, v);
    const int64_t y = xL + math::log(v) / lL;
    if (y < 0)
      return step1();
    v = v * lL * (u - p2);
    return step5_0(y, v);
  }

  // Check right exponential tail
  int64_t step4(const float u, float v) {
    const int64_t y = xR - math::log(v) / lR;
    v = v * lR * (u - p3);
    return step5_0(y, v);
  }

  // Determine appropriate evaluation method
  int64_t step5_0(const int64_t y, const float v) {
    if (M >= 100 && y > 50)
      return step5_2(y, v);
    return step5_1(y, v);
  }

  // Recursive evaluation
  int64_t step5_1(const int64_t y, const float v) {
    float F = 1.0;
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
  int64_t step5_2(const int64_t y, const float v) {
    const float q = (mu - y) / y;
    const float UB =
        y - mu + (y + 0.5) * q * (1 + q * (-0.5 + q / 3)) + 0.00084;
    const float A = math::log(v);
    if (A > UB)
      return step1();
    float D = (y + 0.5) * std::pow(q, 4) / 4;
    if (q < 0)
      D = D / (1 + q);
    if (A < (UB - D - 0.004))
      return y;
    return step5_3(y, A);
  }

  // Accept/reject test
  int64_t step5_3(const int64_t y, const float A) {
    float lnF = (M + 0.5) * math::log(M / mu);
    lnF += (y + 0.5) * math::log(mu / y - M + y);
    lnF += (1.0 / M - 1.0 / y) / 12.0;
    lnF += (1.0 / std::pow(y, 3.0) - 1.0 / std::pow(M, 3.0)) / 360.0;
    if (A > lnF) {
      return step1();
    }
    return y;
  }
};

template <> struct PTPE<CPUFeatures::neon> {
  static int fill(Generator *generator, const int64_t N, int32_t buffer[],
                  const double lambda) {
    assert(N > 0);
    assert(generator != nullptr);
    /*
      PTPE produces valid variates for lambda >= 27.
      For cases where this is not satisied, we use inverse transform.
    */

    if (lambda >= 27) {
      ptpe<CPUFeatures::neon>().run(generator, N, buffer, lambda);
    } else {
      icdf<CPUFeatures::neon>().run(generator, N, buffer, lambda);
    }
    return VSL_ERROR_OK;
  }
};

template <> struct Poisnorm<CPUFeatures::neon> {
  static int fill(Generator *generator, const int64_t N, int32_t buffer[],
                  const double lambda) {
    assert(N > 0);
    assert(generator != nullptr);
    /*
      Use a Gaussian approximation for lambda >= 10.
      For cases where this is not satisfied, we use inverse transform.
    */
    if (lambda >= 10) {
      poisnorm<CPUFeatures::neon>().run(generator, N, buffer, lambda);
    } else {
      icdf<CPUFeatures::neon>().run(generator, N, buffer, lambda);
    }
    return VSL_ERROR_OK;
  }
};
} // namespace openrng::distribution::poisson
