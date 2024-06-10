/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/weibull.hpp>

namespace openrng::distribution::weibull {

/**
 * Creates a Weibull sequence using the Weibull ICDF method.
 *
 * The Weibull ICDF method applies the following transformation to a
 * uniformly distributed variable to generate a random number x.
 *
 *   x = beta * pow(-log(u), 1/alpha) + a
 *
 * The result is stored in r.
 *
 * Assumes n > 0.
 * Assumes generator is a pointer to a valid generator.
 * Assumes r is of length n.
 */
template <openrng::FloatingPoint T>
int fill(const int64_t method, Generator *generator, const int64_t n, T r[],
         const T alpha, const T a, const T beta) {
  if (n < 0 || generator == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (beta <= 0) {
    return VSL_ERROR_BADARGS;
  }

  if (alpha <= 0) {
    return VSL_ERROR_BADARGS;
  }

  if (r == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (n == 0) {
    return VSL_ERROR_OK;
  }

  switch (method) {
  case VSL_RNG_METHOD_WEIBULL_ICDF:
    return choose<ICDF>(generator, n, r, alpha, a, beta);
  case VSL_RNG_METHOD_WEIBULL_ICDF_ACCURATE:
    /*
     * The accurate mode calls the fast mode as
     * no parameters values have been identified
     * that would return values outside the
     * definitioninal domain.
     */
    return choose<ICDF>(generator, n, r, alpha, a, beta);
  default:
    return VSL_ERROR_BADARGS;
  }
}

} // namespace openrng::distribution::weibull
