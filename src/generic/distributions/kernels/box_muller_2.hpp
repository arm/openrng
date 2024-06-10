/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/concepts.hpp>
#include <utils/cpu_features.hpp>

#include <cassert>
#include <cmath>
#include <numbers>

namespace openrng::distribution::gaussian {

template <CPUFeatures> struct BoxMuller2 {};

/**
 * Creates a Gaussian sequence using the Box Muller 2 method.
 *
 * The Box Muller 2 method applies the following transformation to two
 * uniformly distributed variables to generate a pair of Gaussian distributed
 * variables.
 *
 *   x_1 = sqrt(-2 * log(u1)) * sin(2 * std::numbers::pi_v<T> * u2);
 *
 *   x_2 = sqrt(-2 * log(u1)) * cos(2 * std::numbers::pi_v<T> * u2);
 *
 * If x_2 is unused, boxMuller2Fill saves u1 and u2 inside the generator using
 * Generator::storeBoxMuller22State and retrieves this state from the generator
 * using Generator::getAndResetBoxMuller22State on subsequent calls, regardless
 * of whether the generator has advanced since the previous call.
 *
 * Assumes n > 0.
 * Assumes generator is a pointer to a valid generator.
 * Assumes r is of length n.
 */
template <> struct BoxMuller2<CPUFeatures::generic> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[], const T a,
                  const T sigma) {
    assert(n > 0);
    assert(generator != nullptr);

    // First check if state from a previous call has been stored inside the
    // generator. If so, we use the second term of Box Muller 2.
    const auto previousState = generator->getAndResetBoxMuller2State<T>();
    uint64_t valuesRemaining = n;
    if (previousState) {
      const auto [u1, u2] = *previousState;
      r[0] = sigma * std::sqrt(-2 * std::log(u1)) *
                 std::cos(2 * std::numbers::pi_v<T> * u2) +
             a;
      valuesRemaining -= 1;
    }

    // If the user only requested one element, and there had been state saved,
    // then we can exit early.
    if (valuesRemaining == 0) {
      return VSL_ERROR_OK;
    }

    // We're guaranteed &r[0] is safe because n > 0. If there's previous state
    // and we got here, then n > 1 meaning &r[1] is safe.
    T *const adjustedBase = previousState ? &r[1] : &r[0];
    const uint64_t evenValuesRemaining = valuesRemaining & ~1;
    const bool needOneMore = valuesRemaining & 1;

    // Fill all pairs using the Box Muller 2 method.
    generator->fill(evenValuesRemaining, adjustedBase);
    for (uint64_t i = 0; i < evenValuesRemaining; i += 2) {
      const auto u1 = adjustedBase[i];
      const auto u2 = adjustedBase[i + 1];
      adjustedBase[i + 0] = sigma * std::sqrt(-2 * std::log(u1)) *
                                std::sin(2 * std::numbers::pi_v<T> * u2) +
                            a;
      adjustedBase[i + 1] = sigma * std::sqrt(-2 * std::log(u1)) *
                                std::cos(2 * std::numbers::pi_v<T> * u2) +
                            a;
    }

    // If the remaining space in the array was even, all pairs fit perfectly.
    if (!needOneMore) {
      return VSL_ERROR_OK;
    }

    // If we didn't have space for the last pair, fill the remaining space in
    // the output buffer using the 1st term of Box Muller 2, and save the state
    // for later.
    T remainingBuffer[2];
    generator->fill(2, remainingBuffer);

    const auto [u1, u2] = remainingBuffer;
    r[n - 1] = sigma * std::sqrt(-2 * std::log(u1)) *
                   std::sin(2 * std::numbers::pi_v<T> * u2) +
               a;
    generator->storeBoxMuller2State<T>({u1, u2});

    return VSL_ERROR_OK;
  }
};
} // namespace openrng::distribution::gaussian
