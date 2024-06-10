/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <arm_sve.h>

namespace openrng::math::sve {

template <int N> inline svfloat32_t horner(svfloat32_t x, const float poly[]) {
  const auto pg = svptrue_b32();
  auto p = svmla_x(pg, svdup_f32(poly[N - 1]), x, poly[N]);
  for (int i = N - 2; i >= 0; i--) {
    p = svmad_x(pg, x, p, poly[i]);
  }
  return p;
}

template <int N> inline svfloat64_t horner(svfloat64_t x, const double poly[]) {
  const auto pg = svptrue_b64();
  auto p = svmla_x(pg, svdup_f64(poly[N - 1]), x, poly[N]);
  for (int i = N - 2; i >= 0; i--) {
    p = svmad_x(pg, x, p, poly[i]);
  }
  return p;
}

} // namespace openrng::math::sve
