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

namespace openrng::distribution::exponential {

/**
 * Creates an Exponential sequence using the Exponential ICDF method.
 *
 * Exponential ICDF method applies the following transformation to a uniformly
 * distributed variable to generate a random number
 *
 *   x = -beta * log(u) + a;
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

    generator->fill(n, r);

    for (int64_t i = 0; i < n; i++) {
      r[i] = -beta * std::log(r[i]) + a;
    }

    return VSL_ERROR_OK;
  }
};
} // namespace openrng::distribution::exponential
