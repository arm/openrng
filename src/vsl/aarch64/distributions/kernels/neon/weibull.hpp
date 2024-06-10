/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/weibull.hpp>
#include <math/log.hpp>

namespace openrng::distribution::weibull {
template <> struct ICDF<CPUFeatures::neon> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[], const T alpha,
                  const T a, const T beta) {
    assert(n > 0);
    assert(generator != nullptr);

    generator->fill(n, r);

    for (int64_t i = 0; i < n; i++) {
      r[i] = beta * std::pow(-math::log(r[i]), 1 / alpha) + a;
    }

    return VSL_ERROR_OK;
  }
};
} // namespace openrng::distribution::weibull