/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/gamma.hpp>
#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/choose_distribution_kernel.hpp>
#include <utils/concepts.hpp>

#include <cassert>
#include <cmath>

namespace openrng::distribution::gamma {

template <openrng::FloatingPoint T>
int fill(const int64_t method, Generator *generator, const int64_t n, T r[],
         const T alpha, const T a, const T beta) {

  if (n < 0 || generator == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (alpha <= 0) {
    return VSL_ERROR_BADARGS;
  }

  if (beta <= 0) {
    return VSL_ERROR_BADARGS;
  }

  if (r == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (n == 0) {
    return VSL_ERROR_OK;
  }

  switch (method) {
  /*
   * GNORM_ACCURATE and GNORM methods call the same methodologies as no
   * parameters values have been identified that would return values
   * outside the definitional domain for which the GNORM_ACCURATE method
   * would guarantee values of a gamma distribution within the definitional
   * domain.
   */
  case VSL_RNG_METHOD_GAMMA_GNORM:
  case VSL_RNG_METHOD_GAMMA_GNORM_ACCURATE: {
    if (alpha > 1) {
      return choose<GNorm>(generator, n, r, alpha, a, beta);
    } else if (alpha == 1) {
      return choose<openrng::distribution::exponential::ICDF>(generator, n, r,
                                                              a, beta);
    } else if (alpha < 0.6) {
      return choose<GNorm_AD>(generator, n, r, alpha, a, beta);
    } else if (alpha >= 0.6 && alpha < 1) {
      return choose<GNorm_RW>(generator, n, r, alpha, a, beta);
    }
    return VSL_ERROR_BADARGS;
  }
  default:
    return VSL_ERROR_BADARGS;
  }
}

} // namespace openrng::distribution::gamma
