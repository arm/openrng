/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/binomial.hpp>
#include <math/log.hpp>

namespace openrng::distribution::binomial {

/**
 * Implements the inverse transformation with table lookup.
 */
template <> struct icdf<CPUFeatures::neon> {
  void run(Generator *generator, const int64_t N, int32_t *buffer,
           const int64_t n, const double p) {
    static_assert(sizeof(*buffer) == sizeof(float));
    auto *floatView = reinterpret_cast<float *>(buffer);
    generator->fill(N, floatView);

    const int mean = std::ceil(n * std::min(1.0 - p, p));
    const int s = std::ceil(std::sqrt(n * p * (1.0 - p)));

    /*
      Expected number of outputs outside 15 standard deviations of the
      mean is ~10^-50
    */
    const int M = std::min((int)n, mean + 15 * s - 1);
    DynamicArray<float> H(M + 2);
    calculateH(n, p, M, H.data());
    const int K = 10 * M;
    DynamicArray<int> A(K);
    calculateA(K, H.data(), A.data());
    const int L = K - 1;

    if (p > 0.5) {
      for (int i = 0; i < N; i++) {
        buffer[i] =
            n - generateBinomialVariate(A.data(), H.data(), L, floatView[i]);
      }
    } else {
      for (int i = 0; i < N; i++) {
        buffer[i] =
            generateBinomialVariate(A.data(), H.data(), L, floatView[i]);
      }
    }
  }

  inline int64_t generateBinomialVariate(const int *A, const float *H,
                                         const int L, double u) {
    int x = A[(int)(L * u)];

    /*
      Table A at most underestimates the actual output, never overestimates.
      Hence, we only need to check if adjustment up is needed.
    */
    while (u >= H[x + 1]) {
      x++;
    }

    return x;
  }

  inline void calculateH(const int64_t n, const double p, const int M,
                         float *H) {
    const double r = std::min(p, 1.0 - p);
    const double q = 1.0 - r;
    const double r_q = r / q;
    const double s = std::pow(q, n);
    const double g = r_q * (n + 1);

    H[0] = 0;
    H[1] = s;
    double lastP = s;
    for (int x = 2; x < M - 1; x++) {
      lastP = lastP * ((g / (x - 1)) - r_q);
      H[x] = H[x - 1] + lastP;
    }

    /*
      Outputs of the generator should be 0 <= u < 1, but in practice
      u = 1 is possible with some generators. If H[M - 1] = 1.0 (which
      it should since it represents a binomial CMF), then we would get
      a segfault for u=1 in generateBinomialVariate by trying to access
      H[M]. Checking for u = 1.0 explicitly adds unnecessary overhead,
      so instead we set H[M - 1] slightly above 1.0, removing the risk
      of segfault without impacting the output of binomial ICDF.
    */
    H[M - 1] = 1.001;
  }

  inline void calculateA(const int K, const float *H, int *A) {
    const double s = K - 1;
    int i = 0;
    for (int j = 0; j < K; j++) {
      const double x = j / s;
      while (!(H[i] <= x && x <= H[i + 1])) {
        i++;
      }
      A[j] = i;
    }
  }
};

/**
 * BTPE (Binomial, Triangle, Parallelogram, Exponential)
 * generates binomial elements using accept/reject approach.
 *
 * Based on:
 * Kachitvichyanukul, Voratas and Schmeise, Bruce W.,
 * "Binomial Random Variate Generation",
 * Communications of the ACM Vol. 31, 1988,
 * https://dl.acm.org/doi/pdf/10.1145/42372.42381
 */
template <> struct btpe<CPUFeatures::neon> {
  Generator *generator;
  int64_t n;
  double p;
  double r;
  double nrq;
  double fM;
  int M;
  double p1;
  double xL;
  double xR;
  double c;
  double lL;
  double lR;
  double p2;
  double p3;
  double p4;

  static const int nVariates = 100;
  double inputVariates[nVariates];
  int jInput;

  void run(Generator *generator_, const int64_t N, int32_t *buffer,
           const int64_t n_, const double p_) {
    generator = generator_;
    n = n_;
    p = p_;
    r = std::min(p, 1.0 - p);
    nrq = n * r * (1.0 - r);
    fM = n * r + r;
    M = fM;
    p1 = std::floor(2.195 * std::sqrt(nrq) - 4.6 * (1.0 - r)) + 0.5;
    xL = M + 0.5 - p1;
    xR = M + 0.5 + p1;
    c = 0.134 + (20.5 / (15.3 + M));
    lL = (fM - xL) / (fM - xL * r) * (1.0 + 0.5 * (fM - xL) / (fM - xL * r));
    lR = (xR - fM) / (xR * (1.0 - r)) *
         (1.0 + 0.5 * (xR - fM) / (xR * (1.0 - r)));
    p2 = p1 * (1.0 + c + c);
    p3 = p2 + (c / lL);
    p4 = p3 + (c / lR);
    jInput = nVariates;

    for (int i = 0; i < N; i++) {
      buffer[i] = step1();
    }
  }

  inline void refreshInputVariates() {
    if (jInput == nVariates) {
      generator->fill(nVariates, inputVariates);
      jInput = 0;
    }
  }

  /*
    Generate new variates if input buffer is exhausted
    and check triangle region.
  */
  inline int64_t step1() {
    refreshInputVariates();
    const double u = inputVariates[jInput] * p4;
    const double v = inputVariates[jInput + 1];
    jInput += 2;
    if (u > p1)
      return step2(u, v);
    const int64_t y = M + 0.5 - p1 * v + u;
    return step6(y);
  }

  // Check parallelogram region
  inline int64_t step2(const double u, double v) {
    if (u > p2)
      return step3(u, v);
    const double x = xL + (u - p1) / c;
    v = v * c + 1.0 - std::abs(M - x + 0.5) / p1;
    if (v > 1.0 || v <= 0.0)
      return step1();
    const int64_t y = x;
    return step5_0(y, v);
  }

  // Check left exponential tail
  inline int64_t step3(const double u, double v) {
    if (u > p3)
      return step4(u, v);
    const int64_t y = xL + math::log(v) / lL;
    if (y < 0)
      return step1();
    v = v * lL * (u - p2);
    return step5_0(y, v);
  }

  // Check right exponential tail
  inline int64_t step4(const double u, double v) {
    const int64_t y = xR - math::log(v) / lR;
    if (y > n)
      return step1();
    v = v * lR * (u - p3);
    return step5_0(y, v);
  }

  // Determine appropriate evaluation method
  inline int64_t step5_0(const int64_t y, const double v) {
    const double k = std::abs(y - M);
    if (k > 20.0 && k < (0.5 * nrq - 1.0))
      return step5_2(y, v, k);
    return step5_1(y, v);
  }

  // Recursive evaluation
  inline int64_t step5_1(const int64_t y, const double v) {
    const double s = r / (1.0 - r);
    const double a = s * (n + 1);
    double F = 1.0;
    if (M < y) {
      for (int j = M + 1; j <= y; j++) {
        F = F * (a / j - s);
      }
    } else if (M > y) {
      for (int j = y + 1; j <= M; j++) {
        F = F / (a / j - s);
      }
    }
    if (v > F)
      return step1();

    return step6(y);
  }

  // Squeezing
  inline int64_t step5_2(const int64_t y, const double v, const double k) {
    const double rho =
        (k / nrq) * ((k * (k / 3.0 + 0.625) + (1.0 / 6.0)) / nrq + 0.5);
    const double t = -(k * k) / (2.0 * nrq);
    const double A = math::log(v);
    if (A < (t - rho))
      return step6(y);
    if (A > (t + rho))
      return step1();
    return step5_3(y, A);
  }

  // Accept/reject test
  inline int64_t step5_3(const int64_t y, const double A) {
    const double x1 = y + 1;
    const double f1 = M + 1;
    const double z1 = n + 1 - M;
    const double w1 = n - y + 1;
    const double f2 = f1 * f1;
    const double w2 = w1 * w1;
    const double x2 = x1 * x1;
    const double z2 = z1 * z1;

    if (A >
        ((M + 0.5) * math::log(f1 / x1) + (n - M + 0.5) * math::log(z1 / w1) +
         (y - M) * math::log((w1 * r) / (x1 * (1.0 - r))) +
         (13860. - (462. - (132. - (99. - 140. / f2) / f2) / f2) / f2) / f1 /
             166320. +
         (13860. - (462. - (132. - (99. - 140. / w2) / w2) / w2) / w2) / w1 /
             166320. +
         (13860. - (462. - (132. - (99. - 140. / x2) / x2) / x2) / x2) / x1 /
             166320. +
         (13860. - (462. - (132. - (99. - 140. / z2) / z2) / z2) / z2) / z1 /
             166320.)) {
      return step1();
    }
    return step6(y);
  }

  // Output binomial variate
  inline int64_t step6(int64_t y) {
    if (p > 0.5)
      y = n - y;
    return y;
  };
};

template <> struct BTPE<CPUFeatures::neon> {
  static int fill(Generator *generator, const int64_t N, int32_t buffer[],
                  const int64_t n, const double p) {
    assert(N > 0);
    assert(generator != nullptr);

    /*
      BTPE produces valid variates for n*min(1-p,p)>30.
      For cases where this is not satisied, we use inverse transform.
    */
    if (n * std::min(p, 1.0 - p) > 30) {
      btpe<CPUFeatures::neon>().run(generator, N, buffer, n, p);
    } else {
      icdf<CPUFeatures::neon>().run(generator, N, buffer, n, p);
    }

    return VSL_ERROR_OK;
  }
};
} // namespace openrng::distribution::binomial
