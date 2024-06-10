/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

/**
 * @file
 * @brief Inverse error function implementations (SVE).
 *
 * SVE implementations of the inverse error function, in both single-
 * and double-precision, adopted from Arm Optimized Routines.
 */

#pragma once

#include <math/sve/erfinv_data.hpp>
#include <math/sve/log.hpp>
#include <math/sve/util.hpp>

#include <arm_sve.h>

namespace openrng::math::sve {
namespace {

inline svfloat32_t special(svbool_t pg, svfloat32_t x) {
  using d = erfinv_data<float>;
  const auto ax = svabs_x(pg, x);
  const auto t = svdivr_x(
      pg, svsqrt_x(pg, svneg_x(pg, math::sve::log(svsubr_x(pg, ax, 1), pg))),
      1);

  const auto sign = sveor_x(pg, svreinterpret_u32(ax), svreinterpret_u32(x));
  const auto ts = svreinterpret_f32(svorr_x(pg, sign, svreinterpret_u32(t)));
  const auto q =
      svmla_x(pg, svdup_f32(d::Q_50[0]), svadd_x(pg, t, d::Q_50[1]), t);

  return svdiv_x(pg, horner<5>(t, d::P_50), svmul_x(pg, ts, q));
}

inline svfloat64_t special(svbool_t pg, svfloat64_t x) {
  using d = erfinv_data<double>;
  const auto ax = svabs_x(pg, x);
  auto t = svneg_x(pg, math::sve::log(svsubr_x(pg, ax, 1), pg));
  t = svdivr_x(pg, svsqrt_x(pg, t), 1);
  const auto sign = sveor_x(pg, svreinterpret_u64(ax), svreinterpret_u64(x));
  const auto ts = svreinterpret_f64(svorr_x(pg, sign, svreinterpret_u64(t)));

  auto q = svadd_x(pg, t, d::Q_57[8]);
  for (int i = 7; i >= 0; i--)
    q = svmad_x(pg, q, t, d::Q_57[i]);

  return svdiv_x(pg, horner<8>(t, d::P_57), svmul_x(pg, ts, q));
}

inline svfloat64_t lookup(const double *c, svuint64_t idx) {
  return svtbl(svld1rq_f64(svptrue_b64(), c), idx);
}

} // namespace

inline svfloat32_t erfinv(svfloat32_t x, svbool_t pg) {
  using d = erfinv_data<float>;

  const auto is_tail = svacge(pg, x, 0.75);
  const auto extreme_tail = svacge(pg, x, 0.9375);

  const auto t = svmla_x(
      pg, svsel(is_tail, svdup_f32(d::tailshift), svdup_f32(-0.5625)), x, x);

  const auto idx = svdup_u32_z(is_tail, 1);
  const auto idxhi = svadd_x(pg, idx, 2);

  const auto pqhi = svld1rq(svptrue_b32(), &d::coeffs[0]);
  const auto plo = svld1rq(svptrue_b32(), &d::coeffs[4]);
  const auto qlo = svld1rq(svptrue_b32(), &d::coeffs[8]);

  const auto p2 = svtbl(pqhi, idx);
  const auto p1 = svtbl(plo, idxhi);
  const auto p0 = svtbl(plo, idx);
  const auto q0 = svtbl(qlo, idx);
  const auto q1 = svtbl(qlo, idxhi);
  const auto q2 = svtbl(pqhi, idxhi);

  auto p = svmla_x(pg, p1, p2, t);
  p = svmla_x(pg, p0, p, t);
  p = svmad_m(is_tail, p, t, d::P29_0);
  auto y = svmul_x(pg, x, p);

  auto q = svadd_x(pg, t, q2);
  q = svmla_x(pg, q1, q, t);
  q = svmla_x(pg, q0, q, t);

  if ((svptest_any(pg, extreme_tail))) [[unlikely]] {
    return svsel(extreme_tail, special(extreme_tail, x), svdiv_x(pg, y, q));
  }
  return svdiv_x(pg, y, q);
}

inline svfloat64_t erfinv(svfloat64_t x, svbool_t pg) {
  using d = erfinv_data<double>;

  const auto is_tail = svacgt(pg, x, 0.75);
  const auto extreme_tail = svacgt(pg, x, 0.9375);
  const auto idx = svdup_n_u64_z(is_tail, 1);

  auto t = svsel_f64(is_tail, svdup_f64(d::tailshift), svdup_f64(-0.5625));
  t = svmla_x(pg, t, x, x);

  auto p = lookup(&d::P[6][0], idx);
  auto q = svmla_x(pg, lookup(&d::Q[6][0], idx), svdup_n_f64_z(is_tail, 1), t);

  for (int i = 5; i >= 0; i--) {
    p = svmla_x(pg, lookup(&d::P[i][0], idx), p, t);
    q = svmla_x(pg, lookup(&d::Q[i][0], idx), q, t);
  }

  p = svmad_m(is_tail, p, t, d::P37_0);
  p = svmul_x(pg, p, x);

  if (svptest_any(pg, extreme_tail)) [[unlikely]] {
    return svsel(extreme_tail, special(pg, x), svdiv_x(pg, p, q));
  }
  return svdiv_x(pg, p, q);
}

} // namespace openrng::math::sve
