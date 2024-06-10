/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <cassert>
#include <concepts>
#include <cstdint>
#include <tuple>

namespace openrng {

/**
 * Returns the modulus. The % operator returns the remainder of a division,
 * which differs from the modulus for negative numbers.
 */
template <std::integral T> constexpr static inline T mod(const T a, const T b) {
  return (a % b + b) % b;
}

/**
 * Returns the modulus of base to the exponent, ie.
 *
 *  r = (base ^ exponent) % modulus.
 *
 * This method scales with log(exponent).
 */
template <std::unsigned_integral T>
constexpr static inline T powMod(const T base, T exponent, const T modulus) {
  // The algorithm can be found by searching for "modular exponentiation by
  // squaring" or "right to left binary method of modular exponentiation".
  T result = 1;
  T square = base;

  while (exponent > 0) {
    if (exponent & 1) {
      result = (result * square) % modulus;
    }
    square = (square * square) % modulus;
    exponent >>= 1;
  }
  return result;
}

/**
 * Calculates the inverse of a under modulus m.
 *
 * Solves the following for b
 *
 *   a * b % m == 1
 *
 * Assumes m is prime.
 */
constexpr static inline uint64_t invMod(const uint64_t a, const uint64_t m) {
  // Uses Fermat's little theorem, which states
  //
  //   a = a^m mod m
  //
  // when m is prime. By multiplying both sides by a^-2 we get the identity
  //
  //   a^-1 = a^(m-2) mod m
  //
  return powMod(a, m - 2, m);
}

} // namespace openrng
