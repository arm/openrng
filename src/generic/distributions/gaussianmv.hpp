/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/gaussianmv.hpp>
#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/choose_distribution_kernel.hpp>
#include <utils/concepts.hpp>

namespace openrng::distribution::gaussianmv {

template <openrng::FloatingPoint T>
int fill(const int64_t method, Generator *generator, const int64_t n,
         const int64_t dimen, const int64_t mstorage, T r[], const T *a,
         const T *t) {
  if (n < 0 || dimen < 1 || generator == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (r == nullptr || a == nullptr || t == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (mstorage != VSL_MATRIX_STORAGE_FULL &&
      mstorage != VSL_MATRIX_STORAGE_PACKED &&
      mstorage != VSL_MATRIX_STORAGE_DIAGONAL) {
    return VSL_ERROR_BADARGS;
  }

  if (method != VSL_RNG_METHOD_GAUSSIANMV_BOXMULLER &&
      method != VSL_RNG_METHOD_GAUSSIANMV_BOXMULLER2 &&
      method != VSL_RNG_METHOD_GAUSSIANMV_ICDF) {
    return VSL_ERROR_BADARGS;
  }

  if (n == 0) {
    return VSL_ERROR_OK;
  }

  return choose<GaussianMV>(method, generator, n, r, dimen, mstorage, a, t);
}

} // namespace openrng::distribution::gaussianmv
