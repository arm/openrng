/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <cstdint>

namespace openrng {
constexpr int sobol_max_dimensions = 40;
// The direction numbers array is generated offline by
// generate_direction_nums.py
extern const uint32_t sobol_default_direction_numbers[32][sobol_max_dimensions];
} // namespace openrng
