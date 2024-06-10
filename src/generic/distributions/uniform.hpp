/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <algorithm>
#include <generator/generator.hpp>
#include <openrng.h>
#include <type_traits>
#include <utils/concepts.hpp>

#include <cassert>

namespace openrng::distribution::uniform {

enum class DistributionMode { Fast, Accurate };

/**
 * Generates a uniformly distributed sequence in [a, b).
 *
 * This applies the following scale & shift operation on a
 * random variable uniformly distributed on [0, 1):
 *
 *   x = u * (b - a) + a;
 *
 * The result is stored in r.
 *
 * Assumes n > 0.
 * Assumes generator is a pointer to a valid generator.
 * Assumes r is of length n.
 */
template <openrng::FloatingPoint T>
int stdFill(Generator *generator, const int64_t n, T r[], const T a,
            const T b) {
  assert(n > 0);
  assert(generator != nullptr);

  auto err = generator->fill(n, r, a, b);
  if (err != VSL_ERROR_FEATURE_NOT_IMPLEMENTED) {
    return err;
  }

  err = generator->fill(n, r);
  for (int64_t i = 0; i < n; i++) {
    r[i] = r[i] * (b - a) + a;
  }

  return VSL_ERROR_OK;
}

/**
 * Generates a uniformly accurate distributed sequence in [a, b).
 *
 * This applies the following scale & shift operation on a
 * random variable uniformly distributed on [0, 1):
 *
 *   x = u * (b - a) + a;
 *
 * The result is stored in r and guaranteed to be within the
 * definitional domain [a,b).
 *
 * Assumes n > 0.
 * Assumes generator is a pointer to a valid generator.
 * Assumes r is of length n.
 */
template <openrng::FloatingPoint T>
int stdFillAccurate(Generator *generator, const int64_t n, T r[], const T a,
                    const T b) {
  assert(n > 0);
  assert(generator != nullptr);

  auto err = generator->fill(n, r);
  if (err != VSL_ERROR_OK) {
    return err;
  }

  for (int64_t i = 0; i < n; i++) {
    r[i] = r[i] * (b - a) + a;
    r[i] = std::clamp(r[i], a, b);
  }

  return VSL_ERROR_OK;
}

/**
 * Generates an accurate uniformly distributed integers in [a, b).
 *
 * This functionality is not currently supported.
 */
int stdFillAccurate([[maybe_unused]] Generator *generator,
                    [[maybe_unused]] const int64_t n,
                    [[maybe_unused]] int32_t r[],
                    [[maybe_unused]] const int32_t a,
                    [[maybe_unused]] const int32_t b) {
  return VSL_ERROR_BADARGS;
}

/**
 * Generates uniformly distributed integers in [a, b).
 *
 * See stdFill<FloatingPoint>
 */
int stdFill(Generator *generator, const int64_t n, int32_t r[], const int32_t a,
            const int32_t b) {
  static_assert(sizeof(int32_t) == sizeof(float));

  assert(n > 0);
  assert(generator != nullptr);

  generator->fill(n, reinterpret_cast<float *>(r));
  for (int64_t i = 0; i < n; i++) {
    r[i] = reinterpret_cast<float &>(r[i]) * (b - a) + a;
  }

  return VSL_ERROR_OK;
}

template <typename T>
int fill(const int64_t method, Generator *generator, const int64_t n, T r[],
         const T a, const T b) {
  if (n < 0 || generator == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (r == nullptr) {
    return VSL_ERROR_BADARGS;
  }

  if (a >= b) {
    return VSL_ERROR_BADARGS;
  }

  if (n == 0) {
    return VSL_ERROR_OK;
  }

  switch (method) {
  case VSL_RNG_METHOD_UNIFORM_STD:
    return stdFill(generator, n, r, a, b);
  case VSL_RNG_METHOD_UNIFORM_STD_ACCURATE:
    return stdFillAccurate(generator, n, r, a, b);
  default:
    return VSL_ERROR_BADARGS;
  }
}

} // namespace openrng::distribution::uniform
