/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <stdint.h>

namespace openrng {

struct mt2203_param {
  /*
    a: The value a_n which forms the bottom row of the A matrix
    b: First mask used in tempering
    c: Second mask used in tempering
   */
  uint32_t a, b, c;
};

const extern mt2203_param MT2203_PARAMS[6024];

} // namespace openrng
