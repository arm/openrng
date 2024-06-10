/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <arm_neon.h>

namespace openrng::math::neon {

inline int v_any_u64(uint64x2_t x) { return vpaddd_u64(x) != 0; }
inline int v_any_u32(uint32x4_t x) { return v_any_u64(as<uint64x2_t>(x)); }

constexpr inline float32x4_t dup(float x) { return float32x4_t{x, x, x, x}; }
constexpr inline float64x2_t dup(double x) { return float64x2_t{x, x}; }
constexpr inline uint32x4_t dup(uint32_t x) { return uint32x4_t{x, x, x, x}; }
constexpr inline uint64x2_t dup(uint64_t x) { return uint64x2_t{x, x}; }

} // namespace openrng::math::neon
