/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/bernoulli.hpp>
#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/choose_distribution_kernel.hpp>
#include <utils/concepts.hpp>

namespace openrng::distribution::bernoulli {

int fill(const int64_t method, Generator *generator, const int64_t n,
         int32_t r[], const double p) {
  if (n < 0 || generator == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (r == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (p < 0 || p > 1) {
    return VSL_ERROR_BADARGS;
  }

  if (n == 0) {
    return VSL_ERROR_OK;
  }

  switch (method) {
  case VSL_RNG_METHOD_BERNOULLI_ICDF:
    return choose<ICDF>(generator, n, r, p);
  default:
    return VSL_ERROR_BADARGS;
  }
}

} // namespace openrng::distribution::bernoulli
