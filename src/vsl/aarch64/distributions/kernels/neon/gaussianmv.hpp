/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/gaussianmv.hpp>
#include <distributions/kernels/neon/box_muller.hpp>
#include <distributions/kernels/neon/box_muller_2.hpp>
#include <distributions/kernels/neon/gaussian_icdf.hpp>

namespace openrng::distribution::gaussianmv {

template <> struct GaussianMV<CPUFeatures::neon> {

  template <openrng::FloatingPoint T>
  static inline void packed_to_full(T *tt, const T *t, const int64_t dimen) {
    for (int64_t i = 0; i < dimen; i++)
      for (int64_t j = i; j < dimen; j++)
        tt[j * dimen + i] = t[i * (2 * dimen - i - 1) / 2 + j];
  }

  template <openrng::FloatingPoint T>
  static inline void run_diagonal(const int64_t n, T *r, const int64_t dimen,
                                  const T *a, const T *t) {
    for (int64_t i = 0; i < n; i++) {
      for (int64_t j = 0; j < dimen; j++) {
        r[i * dimen + j] = r[i * dimen + j] * t[j] + a[j];
      }
    }
  }

  template <openrng::FloatingPoint T>
  static inline void run_full(const int64_t n, T *r, const int64_t dimen,
                              const T *a, const T *t) {
    int64_t i = 0;
    for (; i + 4 <= n; i += 4) {
      for (int64_t j = dimen - 1; j >= 0; j--) {
        T u[4] = {0};
        for (int64_t k = 0; k <= j; k++) {
          u[0] += r[i * dimen + k] * t[j * dimen + k];
          u[1] += r[(i + 1) * dimen + k] * t[j * dimen + k];
          u[2] += r[(i + 2) * dimen + k] * t[j * dimen + k];
          u[3] += r[(i + 3) * dimen + k] * t[j * dimen + k];
        }
        r[i * dimen + j] = u[0] + a[j];
        r[(i + 1) * dimen + j] = u[1] + a[j];
        r[(i + 2) * dimen + j] = u[2] + a[j];
        r[(i + 3) * dimen + j] = u[3] + a[j];
      }
    }
    for (; i < n; i++) {
      for (int64_t j = dimen - 1; j >= 0; j--) {
        T u = 0;
        for (int64_t k = 0; k <= j; k++) {
          u += r[i * dimen + k] * t[j * dimen + k];
        }
        r[i * dimen + j] = u + a[j];
      }
    }
  }

  template <openrng::FloatingPoint T>
  static int fill(const int64_t method, Generator *generator, const int64_t n,
                  T r[], const int64_t dimen, const int64_t mstorage,
                  const T *a, const T *t) {
    assert(n > 0);
    assert(generator != nullptr);

    switch (method) {
    case VSL_RNG_METHOD_GAUSSIANMV_BOXMULLER:
      gaussian::BoxMuller<CPUFeatures::neon>::fill<T>(generator, n * dimen, r,
                                                      0, 1);
      break;
    case VSL_RNG_METHOD_GAUSSIANMV_BOXMULLER2:
      gaussian::BoxMuller2<CPUFeatures::neon>::fill<T>(generator, n * dimen, r,
                                                       0, 1);
      break;
    case VSL_RNG_METHOD_GAUSSIANMV_ICDF:
      gaussian::ICDF<CPUFeatures::neon>::fill<T>(generator, n * dimen, r, 0, 1);
      break;
    default:
      return VSL_ERROR_BADARGS;
    }

    switch (mstorage) {
    case VSL_MATRIX_STORAGE_PACKED: {
      DynamicArray<T> tt(dimen * dimen);
      packed_to_full(tt.data(), t, dimen);
      run_full(n, r, dimen, a, tt.data());
      return VSL_ERROR_OK;
    }
    case VSL_MATRIX_STORAGE_DIAGONAL:
      run_diagonal(n, r, dimen, a, t);
      return VSL_ERROR_OK;
    default:
      run_full(n, r, dimen, a, t);
      return VSL_ERROR_OK;
    }
  }
};

} // namespace openrng::distribution::gaussianmv
