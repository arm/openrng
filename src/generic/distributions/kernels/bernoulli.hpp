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

namespace openrng::distribution::bernoulli {

template <CPUFeatures> struct ICDF {};

/**
 * Generates a Bernoulli sequence of 0's and 1's.
 *
 * For each u, from a uniform distribution return 1 if u < p else 0.
 *
 * The result is stored in r.
 *
 * Assumes n > 0.
 * Assumes generator is a pointer to a valid generator.
 * Assumes r is of length n.
 */
template <> struct ICDF<CPUFeatures::generic> {
  static int fill(Generator *generator, const int64_t n, int32_t r[],
                  const double p) {
    assert(n > 0);
    assert(generator != nullptr);

    // Reuse the output buffer for storing intermediate floats. Assuming they
    // have the same element size.
    static_assert(sizeof(*r) == sizeof(float));
    auto *floatView = reinterpret_cast<float *>(r);

    const float pFloat = p;

    generator->fill(n, floatView);
    for (int64_t i = 0; i < n; i++) {
      r[i] = floatView[i] < pFloat ? 1 : 0;
    }

    return VSL_ERROR_OK;
  }
};
} // namespace openrng::distribution::bernoulli
