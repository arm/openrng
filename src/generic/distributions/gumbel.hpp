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
#include <utils/concepts.hpp>

#include <cassert>
#include <cmath>

namespace openrng::distribution::gumbel {

/**
 * Creates a Gumbel sequence using the Gumbel ICDF method.
 *
 * The Gumbel ICDF method applies the following transformation to a
 * uniformly distributed variable to generate a random number x.
 *
 * The method used to generate the random number y is
 * VSL_RNG_METHOD_EXPONENTIAL_ICDF with parameters a = 0, beta = 1.
 *
 *   x = beta * log(y) + a
 *
 * The result is stored in r.
 *
 * Assumes n > 0.
 * Assumes generator is a pointer to a valid generator.
 * Assumes r is of length n.
 */
template <std::floating_point T>
int icdfFill(Generator *generator, const int64_t n, T r[], const T a,
             const T beta) {
  assert(n > 0);
  assert(generator != nullptr);

  exponential::ICDF<CPUFeatures::generic>::fill<T>(generator, n, r, 0, 1);

  for (int64_t i = 0; i < n; i++) {
    r[i] = beta * log(r[i]) + a;
  }

  return VSL_ERROR_OK;
}

template <std::floating_point T>
int fill(const int64_t method, Generator *generator, const int64_t n, T r[],
         const T a, const T beta) {
  if (n < 0 || generator == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (beta < 0) {
    return VSL_ERROR_BADARGS;
  }

  if (r == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (n == 0) {
    return VSL_ERROR_OK;
  }

  switch (method) {
  case VSL_RNG_METHOD_GUMBEL_ICDF:
    return icdfFill(generator, n, r, a, beta);
  default:
    return VSL_ERROR_BADARGS;
  }
}

} // namespace openrng::distribution::gumbel
