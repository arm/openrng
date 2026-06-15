/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <cstdint>

namespace openrng {
namespace niederr_default_initialization {

/* This file contains default initialization values for the NIEDERREITER
   generator (base 2). The irreducible monic polynomials over GF(2) are
   calculated in the python script
   tools/niederr_irreducible_polynomials/niederr_irreducible_polynomials.py
   using the Rabin irreducibility test over GF(2).
*/

/** Maximum supported default dimensionality.  */
constexpr static uint32_t dimen = 318;

/** mindeg - the lowest degree of any of the irreducible polynomials that can be
  represented as a 32-bit integer. This is used to check the validity of the
  polynomials represented as integers. We know from the reference article that
  each chosen polynomial must have degree >=1  */
constexpr static uint32_t mindeg = 1;

/** maxdeg - the highest degree of any of the irreducible polynomials that can
  be represented as a 32-bit integer. As the interface does not allow setting a
  maximum degree, we allow the maximum possible. */
constexpr static uint32_t maxdeg = 31;

/* Each element of polynomials encodes an irreducible monic polynomial over
   GF(2):

       P(x) = c_0 + c_1 * x + c_2 * x**2 + ... + c_d * x**d

   bit i of the integer holds c_i (the coefficient of x**i). The leading
   coefficient c_d is always 1, so the degree d can be recovered as
   d = 31 - clz(poly).  */
extern const uint32_t polynomials[dimen];

} // namespace niederr_default_initialization
} // namespace openrng
