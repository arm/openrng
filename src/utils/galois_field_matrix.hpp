/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <utils/galois_field.hpp>
#include <utils/helpers.hpp>

namespace openrng {

/**
 * This class represents a square (NxN) matrix over a Galois Field.
 *
 * @tparam N  number of rows & columns of the matrix
 * @tparam M  order of the Galois Field
 */
template <int N, uint32_t M> class GaloisFieldMatrix {
  using T = GaloisFieldElement<M>;
  using matrix_t = GaloisFieldMatrix<N, M>;
  std::array<T, N * N> data;

public:
  constexpr GaloisFieldMatrix() : data{} {};

  /**
   * Construct a matrix from an N * N array of elements.
   */
  constexpr GaloisFieldMatrix(const std::array<T, N * N> &data_in)
      : data{data_in} {};

  constexpr T &operator()(int i, int j) { return data[i * N + j]; }
  constexpr const T &operator()(int i, int j) const { return data[i * N + j]; }

  /**
   * Construct an identity matrix from this matrix type.
   */
  static matrix_t identity() {
    matrix_t result;
    for (int i = 0; i < N; i++) {
      result(i, i) = 1;
    }
    return result;
  }

  /**
   * Multiply this matrix by another NxN matrix.
   */
  constexpr matrix_t operator*(const matrix_t &other) const {
    matrix_t result;
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        for (int k = 0; k < N; k++) {
          result(i, j) += (*this)(i, k) * other(k, j);
        }
      }
    }
    return result;
  }

  /**
   * Multiply this matrix by a size N vector.
   */
  auto operator*(const std::array<T, N> &other) const {
    std::array<T, N> result;
    for (int i = 0; i < N; i++) {
      result[i] = 0;
      for (int j = 0; j < N; j++) {
        result[i] += (*this)(i, j) * other[j];
      }
    }
    return result;
  }

  /**
   * Raise this matrix to a power via exponentiation by squaring.
   */
  matrix_t pow(uint64_t exponent) const {
    matrix_t base{*this}, result{matrix_t::identity()};
    while (exponent > 0) {
      if (exponent & 1) {
        result = result * base;
      }
      base = base * base;
      exponent >>= 1;
    }
    return result;
  }
};

} // namespace openrng
