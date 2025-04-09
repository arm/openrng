/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <cstring>
#include <generator/neon/philox4x3210_neon.hpp>
#include <utils/neon_intrinsics.hpp>
#include <utils/philox.hpp>

#include <arm_neon.h>

namespace openrng {

namespace {
inline void philox_oneround(uint32x4x4_t &c, uint32x4_t k0, uint32x4_t k1) {
  auto &[R0, L0, R1, L1] = c.val;

  const uint32x4_t D = vdupq_n_u32(0xD2511F53), C = vdupq_n_u32(0xCD9E8D57);

  const uint32x4_t m0_lo = vreinterpretq_u32_u64(vmull_low_u32(R0, D));
  const uint32x4_t m0_hi = vreinterpretq_u32_u64(vmull_high_u32(R0, D));
  const uint32x4_t m1_lo = vreinterpretq_u32_u64(vmull_low_u32(R1, C));
  const uint32x4_t m1_hi = vreinterpretq_u32_u64(vmull_high_u32(R1, C));

  const uint32x4_t lo_0 = vuzp1q_u32(m0_lo, m0_hi),
                   hi_0 = vuzp2q_u32(m0_lo, m0_hi),
                   lo_1 = vuzp1q_u32(m1_lo, m1_hi),
                   hi_1 = vuzp2q_u32(m1_lo, m1_hi);

  R0 = hi_1 ^ k0 ^ L0;
  L0 = lo_1;
  R1 = hi_0 ^ k1 ^ L1;
  L1 = lo_0;
}

inline uint128_t
do_bijection(const QuadCounterBasedState<std::array<uint32_t, 2>> &state) {
  return philox_10(state.k[0], state.k[1], state.c);
}

inline void
do_bijection_12(const QuadCounterBasedState<std::array<uint32_t, 2>> &state,
                uint32x4x4_t &c0, uint32x4x4_t &c1, uint32x4x4_t &c2) {
  uint32x4_t k0 = vdupq_n_u32(state.k[0]), k1 = vdupq_n_u32(state.k[1]);
  const uint32x4_t k0_inc = vdupq_n_u32(0x9E3779B9),
                   k1_inc = vdupq_n_u32(0xBB67AE85);

  for (int i = 0; i < 10; i++) {
    philox_oneround(c0, k0, k1);
    philox_oneround(c1, k0, k1);
    philox_oneround(c2, k0, k1);
    k0 += k0_inc;
    k1 += k1_inc;
  }
}
} // namespace

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int Philox4x3210Generator<CF>::initialise(const int64_t n,
                                          const unsigned int params[]) {
  for (int i = 0; i < 4; i++)
    this->state.c.data.u32[i] = n > i + 2 ? params[i + 2] : 0;

  this->state.k[0] = n == 0 ? 0 : params[0];
  this->state.k[1] = n > 1 ? params[1] : 0;

  this->state.off = 0;

  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int Philox4x3210Generator<CF>::fillBits(int64_t n, uint32_t dst[]) {
  this->template fill_array<uint32_t, do_bijection, do_bijection_12>(n, dst);
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int Philox4x3210Generator<CF>::fill(int64_t n, float dst[]) {
  this->template fill_array<float, do_bijection, do_bijection_12>(n, dst);
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int Philox4x3210Generator<CF>::fill(int64_t n, double dst[]) {
  this->template fill_array<double, do_bijection, do_bijection_12>(n, dst);
  return VSL_ERROR_OK;
}

} // namespace openrng
