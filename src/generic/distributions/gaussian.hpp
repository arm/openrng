/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/box_muller.hpp>
#include <distributions/kernels/box_muller_2.hpp>
#include <distributions/kernels/gaussian_icdf.hpp>
#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/choose_distribution_kernel.hpp>
#include <utils/concepts.hpp>

namespace openrng::distribution::gaussian {

template <openrng::FloatingPoint T>
int fill(const int64_t method, Generator *generator, const int64_t n, T r[],
         const T a, const T sigma) {
  if (n < 0 || generator == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (sigma < 0) {
    return VSL_ERROR_BADARGS;
  }

  if (r == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (n == 0) {
    return VSL_ERROR_OK;
  }

  switch (method) {
  case VSL_RNG_METHOD_GAUSSIAN_BOXMULLER:
    return choose<BoxMuller>(generator, n, r, a, sigma);
  case VSL_RNG_METHOD_GAUSSIAN_BOXMULLER2:
    return choose<BoxMuller2>(generator, n, r, a, sigma);
  case VSL_RNG_METHOD_GAUSSIAN_ICDF:
    return choose<ICDF>(generator, n, r, a, sigma);
  default:
    return VSL_ERROR_BADARGS;
  }
}

} // namespace openrng::distribution::gaussian
