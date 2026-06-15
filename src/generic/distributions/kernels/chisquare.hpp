/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/gamma.hpp>
#include <distributions/kernels/gaussian_icdf.hpp>
#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/concepts.hpp>
#include <utils/cpu_features.hpp>
#include <utils/dynamic_array.hpp>

#include <cassert>
#include <cmath>

namespace openrng::distribution::chisquare {

/**
 * Creates a Chi-Square sequence as the sum of v square values of independent
 * Gaussian variables of mean 0 and variance 1.
 */

template <CPUFeatures> struct Chi2Gaussian {};

template <> struct Chi2Gaussian<CPUFeatures::generic> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[],
                  const int64_t v) {
    assert(n > 0);
    assert(generator != nullptr);

    if (v == 1) {
      gaussian::ICDF<CPUFeatures::generic>::fill(generator, n, r, (T)0, (T)1);
      for (int64_t i = 0; i < n; i++) {
        r[i] *= r[i];
      }
    } else {
      const int64_t numGaussians = 3 * n;
      DynamicArray<T> gaussians(numGaussians);
      gaussian::ICDF<CPUFeatures::generic>::fill(generator, numGaussians,
                                                 gaussians.data(), (T)0, (T)1);
      for (int64_t i = 0; i < n; i++) {
        r[i] = gaussians[i * 3] * gaussians[i * 3] +
               gaussians[i * 3 + 1] * gaussians[i * 3 + 1] +
               gaussians[i * 3 + 2] * gaussians[i * 3 + 2];
      }
    }

    return VSL_ERROR_OK;
  }
};

/**
 * Creates a Chi-Square sequence as the logarithm of the product of
 * uniformly distributed values between 0 and 1.
 */

template <CPUFeatures> struct Chi2Uniform {};

template <> struct Chi2Uniform<CPUFeatures::generic> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[],
                  const int64_t v) {
    assert(n > 0);
    assert(generator != nullptr);

    const int64_t numUniforms = n * v / 2;

    DynamicArray<T> u(numUniforms);
    generator->fill(numUniforms, u.data());

    for (int64_t i = 0; i < n; i++) {
      T sum = 0;
      for (int64_t j = 0; j < v / 2; j++) {
        const T uij = u[i * v / 2 + j] == T{0} ? std::nextafter(T{0}, T{1})
                                               : u[i * v / 2 + j];
        sum += std::log(uij);
      }
      r[i] = -2 * sum;
    }
    return VSL_ERROR_OK;
  }
};

/**
 * Creates a Chi-Square sequence from a Gamma distribution.
 */

template <CPUFeatures> struct Chi2Gamma {};

template <> struct Chi2Gamma<CPUFeatures::generic> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[],
                  const int64_t v) {
    assert(n > 0);
    assert(generator != nullptr);

    const T alpha = static_cast<T>(v) / 2;
    const T a = (T)0;
    const T beta = (T)2;

    gamma::GNorm<CPUFeatures::generic>::fill(generator, n, r, alpha, a, beta);

    return VSL_ERROR_OK;
  }
};

} // namespace openrng::distribution::chisquare