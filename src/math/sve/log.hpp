/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

/**
 * @file
 * @brief log(x) implementations (SVE).
 *
 * SVE implementations of the log(x) function, in both single- and
 * double-precision, adopted from Arm Optimized Routines.
 */

#pragma once

#include <math/log_data.hpp>
#include <math/sve/log_data.hpp>

#include <arm_sve.h>
#include <numbers>

namespace openrng::math::sve {

/**
 * Single-precision log(x).
 */
inline svfloat32_t log(svfloat32_t x, svbool_t pg) {
  using d = log_data<float>;

  auto u = svreinterpret_u32(x);
  u = svsub_x(pg, u, d::off);
  const auto n = svcvt_f32_s32_x(pg, svasr_x(pg, svreinterpret_s32_u32(u), 23));
  u = svand_x(pg, u, d::mantissa_mask);
  u = svadd_x(pg, u, d::off);
  const auto r = svsub_x(pg, svreinterpret_f32(u), 1.0f);

  const auto r2 = svmul_x(pg, r, r);
  const auto p1356 = svld1rq_f32(svptrue_b32(), d::p1356);
  auto p = svmla_lane(svdup_f32(d::p4), r, p1356, 2);
  auto q = svmla_lane(svdup_f32(d::p2), r, p1356, 1);
  auto y = svmla_lane(svdup_f32(d::p0), r, p1356, 0);
  p = svmla_lane(p, r2, p1356, 3);
  q = svmla_x(pg, q, p, r2);
  y = svmla_x(pg, y, q, r2);
  p = svmla_x(pg, r, n, std::numbers::ln2_v<float>);

  return svmla_x(pg, p, y, r2);
}

/**
 * Double-precision log(x).
 *
 * Note: this is approximated using an order-4 polynomial. Reduced
 * accuracy should be expected compared to the Neon implementation,
 * which uses an order-5 polynomial.
 */
inline svfloat64_t log(svfloat64_t x, svbool_t pg) {
  using d = log_data<double>;

  const auto ix = svreinterpret_u64(x);
  const auto tmp = svsub_x(pg, ix, d::off);
  const auto i = svand_x(pg, svlsr_x(pg, tmp, (51 - d::table_bits)),
                         ((1 << d::table_bits) - 1) << 1);
  const auto k =
      svasr_x(pg, svreinterpret_s64(tmp), 52); /* Arithmetic shift. */
  const auto iz = svsub_x(pg, ix, svand_x(pg, tmp, 0xfffULL << 52));
  const auto z = svreinterpret_f64(iz);

  const auto invc = svld1_gather_index(pg, &d::table[0].invc, i);
  const auto logc = svld1_gather_index(pg, &d::table[0].logc, i);

  const auto r = svmad_x(pg, invc, z, -1);
  const auto kd = svcvt_f64_x(pg, k);
  const auto hi = svmla_x(pg, svadd_x(pg, logc, r), kd, std::numbers::ln2);
  const auto r2 = svmul_x(pg, r, r);
  auto y = svmla_x(pg, svdup_f64(d::p[2]), r, svdup_f64(d::p[3]));
  const auto p = svmla_x(pg, svdup_f64(d::p[0]), r, svdup_f64(d::p[1]));

  y = svmla_x(pg, p, r2, y);
  return svmla_x(pg, hi, r2, y);
}
} // namespace openrng::math::sve
