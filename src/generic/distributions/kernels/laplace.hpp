/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/concepts.hpp>
#include <utils/cpu_features.hpp>

#include <cassert>
#include <cmath>

namespace openrng::distribution::laplace {

/**
 * Creates a Laplace sequence using the Laplace ICDF method.
 *
 * The Laplace ICDF method applies the following transformation to two
 * uniformly distributed variables to generate a random number x.
 *
 *   x = -beta * log(u1) + a, where u2 <= 1/2
 *   x = beta * log(u1) + a, where u2 > 1/2
 *
 * The result is stored in r.
 *
 * Assumes n > 0.
 * Assumes generator is a pointer to a valid generator.
 * Assumes r is of length n.
 */

template <CPUFeatures> struct ICDF {};

template <> struct ICDF<CPUFeatures::generic> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[], const T a,
                  const T beta) {
    assert(n > 0);
    assert(generator != nullptr);

    T uniforms[2];
    for (int64_t i = 0; i < n; i++) {
      generator->fill(2, uniforms);
      const auto [u1, u2] = uniforms;

      r[i] = (u2 <= 0.5) ? -beta * std::log(u1) + a : beta * std::log(u1) + a;
    }

    return VSL_ERROR_OK;
  }
};

} // namespace openrng::distribution::laplace
