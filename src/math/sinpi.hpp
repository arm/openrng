/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <math/sinpi_data.hpp>
#include <math/util.hpp>
#include <utils/concepts.hpp>

#include <cstdint>
#include <math.h>
#include <numbers>

namespace openrng::math {
namespace {

inline double sinpi(double x) {
  /* r = x - rint(x). Range reduction to -1/2 .. 1/2.  */
  const double r = x - std::round(x);
  const double y = r * sinpi_poly(r * r);
  /* If r is odd, invert sign.  */
  const uint64_t odd = as<uint64_t>((int64_t)std::round(x)) << 63;
  return as<double>(odd ^ as<uint64_t>(y));
}

inline float sinpi(float x) {
  /* r = x - rint(x). Range reduction to -1/2 .. 1/2.  */
  const float r = x - std::round(x);
  const float y = r * sinpif_poly(r * r);
  /* If r is odd, invert sign.  */
  const uint32_t odd = as<uint32_t>((int32_t)std::round(x)) << 31;
  return as<float>(odd ^ as<uint32_t>(y));
}

} // namespace
} // namespace openrng::math
