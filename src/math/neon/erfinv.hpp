/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

/**
 * @file
 * @brief Inverse error function implementations (Neon).
 *
 * Neon implementations of the inverse error function, in both single-
 * and double-precision, adopted from Arm Optimized Routines.
 */

#pragma once

#include <math/neon/erfinv_data.hpp>
#include <math/neon/log.hpp>
#include <math/neon/util.hpp>
#include <math/util.hpp>

#include <arm_neon.h>

namespace openrng::math::neon {
namespace {

inline float32x4_t lookup(float32x4_t tbl, uint8x16_t idx) {
  return as<float32x4_t>(vqtbl1q_u8(as<uint8x16_t>(tbl), idx));
}

inline float64x2_t lookup(const double *c, uint8x16_t idx) {
  const auto x = vld1q_f64(c);
  return as<float64x2_t>(vqtbl1q_u8(as<uint8x16_t>(x), idx));
}

inline float32x4_t special(float32x4_t x) {
  using d = erfinv_data<float>;
  const auto t =
      1 / vsqrtq_f32(vnegq_f32(log(vsubq_f32(dup(1.0f), vabsq_f32(x)))));
  const auto ts = vbslq_f32(dup(0x7fffffffu), t, x);
  const auto q = vfmaq_f32(d::Q_50[0], vaddq_f32(t, d::Q_50[1]), t);
  return vdivq_f32(horner<5>(t, d::P_50), vmulq_f32(ts, q));
}

inline float64x2_t special(float64x2_t x) {
  using d = erfinv_data<double>;
  const auto t =
      1 / vsqrtq_f64(vnegq_f64(log(vsubq_f64(dup(1.0), vabsq_f64(x)))));
  const auto ts = vbslq_f64(dup(uint64_t{0x7fffffffffffffff}), t, x);
  const auto q = horner<9>(t, d::Q_57);
  return horner<8>(t, d::P_57) / vmulq_f64(ts, q);
}

inline float32x4_t erfinv(float32x4_t x) {
  using d = erfinv_data<float>;

  auto is_tail = vcageq_f32(x, dup(0.75f));
  auto extreme_tail = vcageq_f32(x, dup(0.9375f));

  auto t = vfmaq_f32(vbslq_f32(is_tail, d::tailshift, dup(-0.5625f)), x, x);

  auto off = as<uint8x16_t>(is_tail) & 4;
  auto idx_lo = vaddq_u8(d::idxlo, off);
  auto idx_hi = vaddq_u8(d::idxhi, off);

  auto p3 = as<float32x4_t>(is_tail & as<uint32x4_t>(d::P29_3));
  auto p0 = lookup(d::Plo, idx_lo), p1 = lookup(d::Plo, idx_hi),
       p2 = lookup(d::PQ, idx_lo), q0 = lookup(d::PQ, idx_hi),
       q1 = lookup(d::Qhi, idx_lo), q2 = lookup(d::Qhi, idx_hi);

  auto p = vfmaq_f32(p2, p3, t);
  p = vfmaq_f32(p1, p, t);
  p = vfmaq_f32(p0, p, t);
  p = vmulq_f32(x, p);

  auto q = vfmaq_f32(q1, vaddq_f32(q2, t), t);
  q = vfmaq_f32(q0, q, t);

  if (v_any_u32(extreme_tail)) [[unlikely]]
    return vbslq_f32(extreme_tail, special(x), p / q);

  return p / q;
}

inline float64x2_t erfinv(float64x2_t x) {
  using d = erfinv_data<double>;

  auto is_tail = vcagtq_f64(x, dup(0.75));
  auto extreme_tail = vcagtq_f64(x, dup(0.9375));

  auto off = as<uint8x16_t>(is_tail) & 8;
  auto idx = d::idx + off;

  auto t = vfmaq_f64(vbslq_f64(is_tail, d::tailshift, dup(-0.5625)), x, x);
  auto p = lookup(&d::P[7][0], idx);
  auto q = as<float64x2_t>(is_tail & as<uint64x2_t>(dup(1.0)));

  for (int i = 6; i >= 0; i--) {
    p = vfmaq_f64(lookup(&d::P[i][0], idx), p, t);
    q = vfmaq_f64(lookup(&d::Q[i][0], idx), q, t);
  }
  p *= x;

  if (v_any_u64(extreme_tail)) [[unlikely]] {
    return vbslq_f64(extreme_tail, special(x), p / q);
  }

  return p / q;
}

inline float erfinv(float x) {
  return vgetq_lane_f32(erfinv(float32x4_t{x, 0, 0, 0}), 0);
}

inline double erfinv(double x) {
  return vgetq_lane_f64(erfinv(float64x2_t{x, 0}), 0);
}

} // namespace
} // namespace openrng::math::neon
