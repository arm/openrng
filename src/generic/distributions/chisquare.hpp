/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/chisquare.hpp>
#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/choose_distribution_kernel.hpp>
#include <utils/concepts.hpp>

namespace openrng::distribution::chisquare {

template <openrng::FloatingPoint T>
int fill(const int64_t method, Generator *generator, const int64_t n, T r[],
         const int64_t v) {

  if (n < 0 || generator == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (v <= 0) {
    return VSL_ERROR_BADARGS;
  }

  if (r == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (n == 0) {
    return VSL_ERROR_OK;
  }

  switch (method) {
  case VSL_RNG_METHOD_CHISQUARE_CHI2GAMMA: {
    if (v == 1 || v == 3) {
      if (v == 3 && n > INT64_MAX / 3) {
        return VSL_ERROR_MEM_FAILURE;
      }
      return choose<Chi2Gaussian>(generator, n, r, v);
    } else if (!(v & 1) && v <= 16) {
      if (n > INT64_MAX / (v / 2)) {
        return VSL_ERROR_MEM_FAILURE;
      }
      return choose<Chi2Uniform>(generator, n, r, v);
    } else {
      return choose<Chi2Gamma>(generator, n, r, v);
    }
  }
  default:
    return VSL_ERROR_BADARGS;
  }
}

} // namespace openrng::distribution::chisquare
