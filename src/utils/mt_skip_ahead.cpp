/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <utils/dynamic_array.hpp>
#include <utils/helpers.hpp>
#include <utils/mt_skip_ahead_data.hpp>

#include <array>
#include <cstddef>
#include <cstdint>

namespace {

/**
 * Finds the degree of the polynomial a.
 */
int deg(const DynamicArray<char> &a) {
  for (int i = a.size() - 1; i >= 0; i--) {
    if (a[i]) {
      return i;
    }
  }
  return -1;
}

/**
 * Returns a ^ (b * x ^ j), where a, b are polynomials over GF2.
 *
 * b * x ^ j is equivalent to shifting all coefficients of b by j.
 */
DynamicArray<char> add_shift(DynamicArray<char> a, const DynamicArray<char> &b,
                             size_t j) {
  auto n = std::max(a.size(), b.size() + j);
  a.resize(n, 0);

  //
  // Strict aliasing prevents the compiler from autovectorising this operation
  // which is where the algorithm spends all of its time. We prevent the
  // compiler from reloading the pointer and loop bound by saving them to local
  // variables.
  //
  auto ap = a.data();
  const auto *bp = b.data();
  const auto bsize = b.size();

  for (size_t i = 0; i < bsize; i++) {
    ap[i + j] ^= bp[i];
  }

  a.resize(deg(a) + 1);
  return a;
}

/**
 * Returns a * b, where a, b are polynomials over GF2.
 */
DynamicArray<char> mul(const DynamicArray<char> &a,
                       const DynamicArray<char> &b) {
  const auto n = a.size() + b.size() - 1;
  DynamicArray<char> result(n, 0);

  for (size_t i = 0; i < a.size(); i++) {
    if (a[i]) {
      result = add_shift(std::move(result), b, i);
    }
  }
  return result;
}

/**
 * Returns g % p, where g, p are polynomials over GF2.
 */
DynamicArray<char> mod(DynamicArray<char> g, const DynamicArray<char> &p) {
  auto &result = g;
  while (deg(result) >= deg(p)) {
    auto pos = deg(result) - deg(p);
    result = add_shift(std::move(result), p, pos);
  }
  return result;
}

/**
 * Returns g ^ n % p, where g, p are polynomials over GF2.
 */
[[maybe_unused]] DynamicArray<char> powMod(DynamicArray<char> g, uint64_t power,
                                           const DynamicArray<char> &p) {
  //
  // Use exponentiation by squaring.
  //
  DynamicArray<char> result(1, 1);
  while (power) {
    if (power & 1) {
      result = mod(mul(std::move(result), g), p);
    }
    g = mod(mul(g, g), p);
    power >>= 1;
  }
  return result;
}

/**
 *  Converts indices to a dense polynomial over GF2.
 *
 *  Indices should be an array describing all non-zero coefficients. The
 *  resulting array is an array explcitly stores all coefficients. Currently
 *  each bit is stored as one byte.
 */
static DynamicArray<char> fromIndices(auto indices) {
  const auto order = 1 + indices.back();

  DynamicArray<char> result(order, 0);
  if (order <= 0) {
    return result;
  }
  for (auto index : indices) {
    result[index] = 1;
  }
  return result;
}

/**
 * Inner method calculating the skip ahead polynomial.
 *
 * We want to calculate
 *
 *   x ^ n % p
 *
 * x and p are polynomials over GF2. The modulus operation (%) returns the
 * remainder under polynomial division.
 *
 * Assuming the order of p > 2, for n := 0, the result is {1}, which is
 * equivalent to advancing the generator 0 steps. If n := 1, then the result is
 * {1, 0} which is equivalent to advancing the generator 1 step. n := 2, the
 * result is {1, 0, 0}, which is equivalent to advancing the generator 2 steps.
 * For n > order of p, the polynomial can have more than 1 non-zero, say {1, 1,
 * 0}, which is equivalent to advancing the state once and twice and then adding
 * (xor in GF2) the result to get the state after n iterations.
 */
template <std::array characteristicPolynomialIndices>
DynamicArray<char> gf2SkipAheadPolynomial(uint64_t n) {
  const DynamicArray<char> p = fromIndices(characteristicPolynomialIndices);
  DynamicArray<char> x(2);
  x[0] = 0;
  x[1] = 1;
  return powMod(x, n, p);
}

} // namespace

namespace openrng {

/**
 * Wrapper around gf2SkipAheadPolynomial
 *
 * Convenience wrapper that choses the appropriate characteristic polynomial.
 */

DynamicArray<char> sfmt19937SkipAheadPolynomial(uint64_t n) {
  return gf2SkipAheadPolynomial<
      skipahead::data::sfmt19937CharacteristicPolynomialIndices>(n);
}
} // namespace openrng
