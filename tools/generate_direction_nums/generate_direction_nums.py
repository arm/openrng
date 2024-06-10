#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its affiliates
# <open-source-office@arm.com></text>
#
# SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
#


from copy import copy

#
# This script generates default direction numbers for the SOBOL
# generator, up to 40 dimensions. The initial values and polynomials
# are those used in
#
# Implementing Sobol’s Quasirandom Sequence Generator, Bratley & Fox 1988
#
# This will be referred to as B&F throughout the script. Their
# original Fortran implementation can be found at
# https://www.netlib.org/toms/index.html, algorithm 659 - in this
# script BDSOBOL refers to the Fortran subroutine of the same name
# therein which defines the initial polynomials and m values
#

# Each element of POLY represents a primitive polynomial of order d over GF(2):
#
# P(x) = 1 + a_{d-1} * x + a_{d-2} * x**2 + ... + a_1 * x**(d-1) + x**d
#
# Note that coefficients for x**0 and x**d terms are always 1, and
# also be careful of the indexes for a - they go in a non-intuitive
# order.
#
# Hence P(x) can be represented by a bit-pattern, where a(d - i) is 1
# if bit i is set, 0 otherwise. For example (from B&F section 4.2),
# POLY[9] = 47, or 101111 in binary. Find order = d by one less than
# the length of the bit-pattern (disregarding leading zeroes) - in
# this case d = 5. All bits are 1, except bit 4, which corresponds to
# a_1. Hence the polynomial is: P(x) = x5 + x3 + x2 + x + 1.
# POLY[i] is the polynomial for dimension i - these 40 polynomials are
# the same as those given in BDSOBOL.
POLY = [  1,   3,   7,  11,  13,  19,  25,  37,  59,  47,
         61,  55,  41,  67,  97,  91, 109, 103, 115, 131,
        193, 137, 145, 143, 241, 157, 185, 167, 229, 171,
        213, 191, 253, 203, 211, 239, 247, 285, 369, 299 ]

# 'Seed' values of m_i (explained below) for each of the 40
# dimensions. These are derived from the initial values used in
# BDSOBOL, however there are some small adjustments needed to maintain
# bit-compatibility with other VSL implementations. This is noted on
# affected rows, where BDSOBOL[i] == X means that, in the BDSOBOL
# version of this row, element i (starting from 0) is X.
INIT = [
    [],
    [1],
    [1, 1],
    [1, 3, 7],
    [1, 1, 5],
    [1, 3, 1,  1],
    [1, 1, 3,  7],
    [1, 3, 3,  9,  9],
    [1, 3, 7, 13,  3],
    [1, 1, 5, 11, 27],
    [1, 3, 5,  1, 15],
    [1, 1, 7,  3, 29],
    [1, 3, 7,  7, 21],
    [1, 1, 1,  9, 23,  37],
    [1, 3, 3,  5, 19,  33],
    [1, 1, 3, 13, 11,   7],
    [1, 1, 7, 13, 25,   5],
    [1, 3, 5, 11,  7,  11],
    [1, 1, 1,  3, 13,  39],
    [1, 3, 1, 15, 17,  63,  13],
    [1, 1, 5,  5,  1,  27,  33], # BDSOBOL[5] = 59
    [1, 3, 3,  3, 25,  17, 115],
    [1, 1, 3, 15, 29,  15,  41], # BDSOBOL[2] = 7
    [1, 3, 1,  7,  3,  23,  79],
    [1, 3, 7,  9, 31,  29,  17],
    [1, 1, 5, 13, 11,   3,  29],
    [1, 3, 1,  9,  5,  21, 119], # BDSOBOL[1] = 1
    [1, 1, 3,  1, 23,  13,  75],
    [1, 3, 3, 11, 27,  31,  73], # BDSOBOL[2] = 7
    [1, 1, 7,  7, 19,  25, 105],
    [1, 3, 5,  5, 21,   9,   7], # BDSOBOL[2] = 1
    [1, 1, 1,  15, 5,  49,  59],
    [1, 1, 1,  1,  1,  33,  65], # BDSOBOL[1] = 3
    [1, 3, 5,  15, 17, 19,  21],
    [1, 1, 7,  11, 13, 29,   3],
    [1, 3, 7,  5,   7, 11, 113],
    [1, 1, 5,  3,  15, 19,  61], # BDSOBOL[3] = 11
    [1, 3, 1,  1,   9, 27,  89,  7], # BDSOBOL[1] = 1
    [1, 1, 3,  7,  31, 15,  45, 23],
    [1, 3, 3,  9,   9, 25, 107, 39] # BDSOBOL[4] = 25
]


def clz_u32(x):
    n = 0
    while not (0x80000000 & x):
        x = x << 1
        n += 1
    return n


def generate_for_one_dimension(dim_num):
    if dim_num == 0:
        # Special-case mentioned in B&F section 3.1
        return [0x80000000 >> i for i in range(32)]

    # The following function implements the algorithm described in B&F
    # section 2. The names m, d, a and v refer to the same variables
    # as in that chapter

    # Each dimension (except the first) has a polynomial with order d,
    # and d initial values of m
    poly = POLY[dim_num]
    m = copy(INIT[dim_num])

    d = 31 - clz_u32(poly)

    # Coefficient n of the polynomial
    def a(n):
        # If n is 0 or d, the indexing in get_m has gone wrong
        assert n > 0 and n < d
        return 0 if ((poly >> (d - n)) & 1) == 0 else 1

    # Calculate m_i using the recurrence relation
    def get_m(i):
        m_i = m[i - d] ^ (m[i - d] << d)
        for k in range(1, d):
            m_i = m_i ^ ((a(k) * m[i - k]) << k)
        return m_i

    # Fill in m
    for i in range(len(m), 32):
        m.append(get_m(i))

    # Transform a_i to v_i. In B&F, v is defined as
    # v_i = m_i / (2**i)
    # i.e. some fraction with a power of 2 as the denominator. This is
    # represented using fixed-point format, where the point is between
    # bits 31 and 32. The numbers are stored as 32-bit integers in the
    # generator itself, so the point is to the left of the MSB.
    return [x << (31 - i) for i, x in enumerate(m)]


TEMPLATE = """
/*
 * Default direction numbers for dimensions 1 to 40
 *
 * This file is generated by generate_direction_nums.py
 */

#include <generator/sobol_default_direction_numbers.hpp>

const uint32_t openrng::sobol_default_direction_numbers[][openrng::sobol_max_dimensions] = {{
{}
}};
"""


def format_one_row(nums):
    return "{" + ",".join((hex(num) for num in nums)) + "}"


def print_source():
    direction_nums = [generate_for_one_dimension(i) for i in range(40)]
    # Transpose the array for no strided access
    transposed_dir_nums = map(list, zip(*direction_nums))
    print(TEMPLATE.format(",\n".join((format_one_row(row)
                                      for row in transposed_dir_nums))))


if __name__ == "__main__":
    print_source()
