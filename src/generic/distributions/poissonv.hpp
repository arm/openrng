/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/poissonv.hpp>
#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/choose_distribution_kernel.hpp>
#include <utils/concepts.hpp>

#include <cmath>

namespace openrng::distribution::poissonv {

int fill(const int64_t method, Generator *generator, const int64_t n,
         int32_t r[], const double lambda[]) {
  if (n < 0 || generator == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (r == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (n == 0) {
    return VSL_ERROR_OK;
  }

  if (lambda == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (method != VSL_RNG_METHOD_POISSONV_POISNORM) {
    return VSL_ERROR_BADARGS;
  }

  return choose<PoisnormV>(generator, n, r, lambda);
}

} // namespace openrng::distribution::poissonv
