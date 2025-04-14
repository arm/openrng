/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/laplace.hpp>
#include <utils/dynamic_array.hpp>

#include <math/log.hpp>

namespace openrng::distribution::laplace {

template <> struct ICDF<CPUFeatures::neon> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[], const T a,
                  const T beta) {
    assert(n > 0);
    assert(generator != nullptr);

    const uint64_t size = 2 * n;

    DynamicArray<T> uniforms(size);
    if (uniforms.data() == nullptr) {
      return VSL_ERROR_MEM_FAILURE;
    }

    auto ok = generator->fill(size, uniforms.data());
    if (ok != VSL_ERROR_OK) {
      return ok;
    }

    for (int64_t i = 0; i < n; i++) {
      auto u1 = uniforms[2 * i + 0];
      auto u2 = uniforms[2 * i + 1];
      r[i] = (-2 * std::signbit(u2 - 0.5) + 1) * beta * math::log(u1) + a;
    }

    return VSL_ERROR_OK;
  }
};

} // namespace openrng::distribution::laplace