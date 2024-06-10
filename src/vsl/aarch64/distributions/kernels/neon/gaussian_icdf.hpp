/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/gaussian_icdf.hpp>
#include <math/neon/erfinv.hpp>

namespace openrng::distribution::gaussian {

namespace {
template <openrng::FloatingPoint T> inline void do_one(T *r, T a, T sigma) {
  const auto u = 2 * r[0] - 1;
  r[0] = sigma * std::sqrt(T{2}) * math::neon::erfinv(u) + a;
}

template <openrng::FloatingPoint T> inline void do_vec(T *r, T a, T sigma);

template <> inline void do_vec<float>(float *r, float a, float sigma) {
  const auto u = 2 * vld1q_f32(r) - 1;
  vst1q_f32(r, sigma * std::sqrt(2.0f) * math::neon::erfinv(u) + a);
}

template <> inline void do_vec<double>(double *r, double a, double sigma) {
  const auto u = 2 * vld1q_f64(r) - 1;
  vst1q_f64(r, sigma * std::sqrt(2.0) * math::neon::erfinv(u) + a);
}
} // namespace

template <> struct ICDF<CPUFeatures::neon> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[], const T a,
                  const T sigma) {
    assert(n > 0);
    assert(generator != nullptr);

    generator->fill(n, r);

    constexpr int64_t vec_size = 16 / sizeof(T);

    int64_t i = 0;

    for (; i <= n - vec_size; i += vec_size)
      do_vec(&r[i], a, sigma);

    for (; i < n; i++)
      do_one(&r[i], a, sigma);

    return VSL_ERROR_OK;
  }
};

} // namespace openrng::distribution::gaussian
