/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <utils/dynamic_array.hpp>

#include <cstdint>

namespace openrng {

/**
 *  Determine the skip ahead polynomial to skip n iterations.
 *
 *  The skip ahead polynomial is the polynomial described in equation 4 of
 *
 *    Hiroshi Haramoto, Makoto Matsumoto, Takuji Nishimura, François Panneton,
 *    and Pierre L'Ecuyer. 2008. Efficient Jump Ahead for F2-Linear Random
 *    Number Generators. INFORMS J. on Computing 20, 3 (Summer 2008), 385–390.
 *    https://doi.org/10.1287/ijoc.1070.0251
 */
DynamicArray<char> mt19937SkipAheadPolynomial(uint64_t n);

/**
 *  Determine the skip ahead polynomial to skip n iterations.
 *
 *  See mt19937SkipAheadPolynomial.
 */
DynamicArray<char> sfmt19937SkipAheadPolynomial(uint64_t n);

} // namespace openrng
