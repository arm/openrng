/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <math/log.hpp>

#include <arm_neon.h>

namespace openrng::math::neon {
namespace {
inline float32x4_t log(float32x4_t x) {
  return float32x4_t{
      math::log(vgetq_lane_f32(x, 0)),
      math::log(vgetq_lane_f32(x, 1)),
      math::log(vgetq_lane_f32(x, 2)),
      math::log(vgetq_lane_f32(x, 3)),
  };
}

inline float64x2_t log(float64x2_t x) {
  return float64x2_t{
      math::log(vgetq_lane_f64(x, 0)),
      math::log(vgetq_lane_f64(x, 1)),
  };
}
} // namespace
} // namespace openrng::math::neon
