/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/exponential.hpp>
#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/choose_distribution_kernel.hpp>
#include <utils/concepts.hpp>

#include <cassert>
#include <cmath>

namespace openrng::distribution::exponential {

/**
 * Creates an Exponential sequence using the Exponential ICDF method.
 *
 * Exponential ICDF method applies the following transformation to a uniformly
 * distributed variable to generate a random number
 *
 *   x = -beta * log(u) + a;
 *
 * The result is stored in r.
 *
 * Assumes n > 0.
 * Assumes generator is a pointer to a valid generator.
 * Assumes r is of length n.
 */
template <openrng::FloatingPoint T>
int fill(const int64_t method, Generator *generator, const int64_t n, T r[],
         const T a, const T beta) {
  if (n < 0 || generator == nullptr) {
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
  case VSL_RNG_METHOD_EXPONENTIAL_ICDF: {
    return choose<ICDF>(generator, n, r, a, beta);
  }
  case VSL_RNG_METHOD_EXPONENTIAL_ICDF_ACCURATE: {
    /*
     * The accurate mode calls the fast mode as
     * no parameters values have been identified
     * that would return values outside the
     * definitioninal domain.
     */
    return choose<ICDF>(generator, n, r, a, beta);
  }
  default:
    return VSL_ERROR_BADARGS;
  }
}

} // namespace openrng::distribution::exponential
