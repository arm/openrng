/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <cstdint>
#include <utils/helpers.hpp>

namespace openrng {

/**
 * An element in a Galois Field of order m
 *
 * A Galois Field defines addition, multiplication and subtraction as the usual
 * results modulus m. Division by a is defined as the value b that solves
 *
 *   a * b % m == 1
 *
 * The public constructor only allows construction with a uint32_t, but all
 * internal arithmetic is done in uint64_t to avoid overflow. It's assumed that
 * m is a prime number.
 */
template <uint32_t m> class GaloisFieldElement {

private:
  constexpr explicit GaloisFieldElement(uint64_t x) : data(x % m) {}

public:
  using Gfe = GaloisFieldElement;

  constexpr GaloisFieldElement() = default;

  constexpr GaloisFieldElement(uint32_t x) : data{x % m} {
    static_assert(sizeof(Gfe) == sizeof(uint32_t));
  }

  constexpr Gfe operator+(Gfe other) const {
    const uint64_t a = data;
    const uint64_t b = other.data;
    return Gfe{a + b};
  }

  constexpr Gfe &operator+=(const Gfe &other) { return *this = *this + other; }

  constexpr Gfe operator-(Gfe other) const {
    const uint64_t a = data;
    const uint64_t b = other.data;

    // Moves the value of a into [m, 2*m) so the result will be in [0, 2*m).
    return Gfe{m + a - b};
  }

  constexpr Gfe &operator-=(const Gfe &other) { return *this = *this - other; }

  constexpr Gfe operator*(Gfe other) const {
    const uint64_t a = data;
    const uint64_t b = other.data;
    return Gfe{a * b};
  }

  constexpr Gfe &operator*=(const Gfe &other) { return *this = *this * other; }

  constexpr Gfe operator/(Gfe other) const {
    const uint64_t a = data;
    const uint64_t b = other.data;
    const uint64_t inv = invMod(b, m);
    return Gfe{a * inv};
  }

  constexpr Gfe &operator/=(const Gfe &other) { return *this = *this / other; }

  constexpr bool operator==(Gfe other) const {
    const uint64_t a = data;
    const uint64_t b = other.data;
    return a == b;
  }

  uint32_t data;
};

} // namespace openrng