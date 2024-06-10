/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <cassert>
#include <cmath>
#include <distributions/kernels/box_muller.hpp>
#include <distributions/kernels/box_muller_2.hpp>
#include <distributions/kernels/gaussian_icdf.hpp>
#include <generator/generator.hpp>
#include <numbers>
#include <openrng.h>

namespace openrng::distribution::lognormal {

template <CPUFeatures> struct BoxMuller2 {};

template <CPUFeatures> struct ICDF {};

/**
 * Lognormal Box Muller 2 distribution.
 *
 * The Lognormal Box Muller 2 applies the following transformation
 *
 *   x = beta * exp(y) + b
 *
 * where y is a normally distributed variable generated from a uniform sequence
 * using the Box Muller 2 method.
 *
 * The result is stored in r.
 *
 * Assumes n > 0.
 * Assumes generator is a pointer to a valid generator.
 * Assumes r is of length n.
 */
template <> struct BoxMuller2<CPUFeatures::generic> {
  template <std::floating_point T>
  static int fill(Generator *generator, const int64_t n, T r[], const T a,
                  const T sigma, const T b, const T beta) {
    assert(n > 0);
    assert(generator != nullptr);
    gaussian::BoxMuller2<CPUFeatures::generic>::fill(generator, n, r, a, sigma);
    for (int64_t i = 0; i < n; i++) {
      T x = beta * std::exp(r[i]) + b;
      r[i] = x;
    }
    return VSL_ERROR_OK;
  }
};

/**
 * Lognormal ICDF distribution.
 *
 * The Lognormal Box Muller 2 applies the following transformation
 *
 *   x = beta * exp(y) + b
 *
 * where y is a normally distributed variable generated from a uniform sequence
 * using the ICDF method.
 *
 * The result is stored in r.
 *
 * Assumes n > 0.
 * Assumes generator is a pointer to a valid generator.
 * Assumes r is of length n.
 */
template <> struct ICDF<CPUFeatures::generic> {
  template <std::floating_point T>
  static int fill(Generator *generator, const int64_t n, T r[], const T a,
                  const T sigma, const T b, const T beta) {
    assert(n > 0);
    assert(generator != nullptr);

    gaussian::ICDF<CPUFeatures::generic>::fill(generator, n, r, a, sigma);
    for (int64_t i = 0; i < n; i++) {
      T x = beta * std::exp(r[i]) + b;
      r[i] = x;
    }

    return VSL_ERROR_OK;
  }
};

} // namespace openrng::distribution::lognormal