/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/geometric.hpp>
#include <math/log.hpp>

#include <cassert>

namespace openrng::distribution::geometric {
template <> struct ICDF<CPUFeatures::neon> {
  static int fill(Generator *generator, const int64_t n, int32_t r[],
                  const double p) {
    assert(n > 0);
    assert(generator != nullptr);

    const int primary_unroll = 16;
    const int tail_unroll = 4;

    static_assert(sizeof(*r) == sizeof(float));
    auto *floatView = reinterpret_cast<float *>(r);

    const double g = 1. / math::log(1. - p);

    generator->fill(n, floatView);

    int i = 0;
    for (; i <= n - primary_unroll; i += primary_unroll) {
      for (int j = i; j < i + primary_unroll; j++) {
        r[j] = math::log(floatView[j]) * g;
      }
    }
    for (; i <= n - tail_unroll; i += tail_unroll) {
      for (int j = i; j < i + tail_unroll; j++) {
        r[j] = math::log(floatView[j]) * g;
      }
    }
    for (; i < n; i++) {
      r[i] = math::log(floatView[i]) * g;
    }

    return VSL_ERROR_OK;
  }
};
} // namespace openrng::distribution::geometric
