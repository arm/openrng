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

inline void philox_10(uint32x4x4_t &c0, uint32x4x4_t &c1, uint32x4x4_t &c2,
                      uint32_t k0_, uint32_t k1_) {
  uint32x4_t k0 = vdupq_n_u32(k0_), k1 = vdupq_n_u32(k1_);
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

void write_elems(uint32_t *dst, const uint32_t *src, size_t n) {
  memcpy(dst, src, n * sizeof(uint32_t));
}

template <std::floating_point T>
void write_elems(T *dst, const uint32_t *src, size_t n) {
  for (unsigned i = 0; i < n; i++)
    dst[i] = (int)src[i] / 0x1p32 + 0.5;
}

float32x4_t transform_to_float(uint32x4_t x) {
  return vfmaq_f32(vdupq_n_f32(0.5), vcvtq_f32_s32(vreinterpretq_s32_u32(x)),
                   vdupq_n_f32(0x1p-32));
}

float32x4x4_t transform_to_float(uint32x4x4_t x) {
  float32x4x4_t f;
  for (int i = 0; i < 4; i++)
    f.val[i] = transform_to_float(x.val[i]);
  return f;
}

void st4(uint32_t *dst, uint32x4x4_t x) { vst4q_u32(dst, x); }

void st4(float *dst, uint32x4x4_t x) { vst4q_f32(dst, transform_to_float(x)); }

void st4(double *dst, uint32x4x4_t x) {
  const float32x4x4_t f = transform_to_float(x);
  float64x2x4_t lo, hi;
  for (int i = 0; i < 4; i++) {
    lo.val[i] = vcvt_f64_f32(vget_low_f32(f.val[i]));
    hi.val[i] = vcvt_high_f64_f32(f.val[i]);
  }
  vst4q_f64(dst, lo);
  vst4q_f64(dst + 8, hi);
}

} // namespace

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int Philox4x3210Generator<CF>::initialise(const int64_t n,
                                          const unsigned int params[]) {
  for (int i = 0; i < 4; i++)
    state.c.data.u32[i] = n > i + 2 ? params[i + 2] : 0;

  state.k0 = n == 0 ? 0 : params[0];
  state.k1 = n > 1 ? params[1] : 0;

  state.off = 0;

  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
void Philox4x3210Generator<CF>::advance(uint64_t by) {
  /* Every 4 advances, c is incremented. If by is a multiple of 4 then we can
   * just bump c and be done. If not, bump c by the number of full quadwords
   * that this advance would skip over, and adjust offset such that when we
   * write to the array we begin from the correct subelement of philox(c).  */
  state.c.data.u128 += (state.off + by) / 4;
  state.off = (state.off + by) % 4;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
uint128_t Philox4x3210Generator<CF>::get_philox() const {
  return philox_10(state.k0, state.k1, state.c);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
template <typename T>
void Philox4x3210Generator<CF>::fill_array(int64_t n, T *dst) {
  /* Walking over the array has at most 3 stages: head, body and tail.  */
  int64_t i = 0;
  uint128_t c = get_philox();

  /* Head: exhaust the current value of c. Starting from current offset into
   * c, write either the remainder of the quadword or n elements (whichever is
   * less). If offset is already zero then this will be a no-op.  */
  const int64_t chunksize = std::min(static_cast<int64_t>(4) - state.off, n);
  write_elems(dst, c.data.u32 + state.off, chunksize);
  advance(chunksize);
  i += chunksize;

  /* Unrolled body - unroll by a factor of 12.  */
  for (; i + 48 < n; i += 48) {
    __uint128_t x[12];
    for (int j = 0; j < 12; j++)
      x[j] = state.c.data.u128 + j;

    uint32x4x4_t x0 = vld4q_u32((uint32_t *)&x[0]),
                 x1 = vld4q_u32((uint32_t *)&x[4]),
                 x2 = vld4q_u32((uint32_t *)&x[8]);
    philox_10(x0, x1, x2, state.k0, state.k1);
    st4(dst + i, x0);
    st4(dst + i + 16, x1);
    st4(dst + i + 32, x2);
    advance(48);
  }

  /* Body: while there are at least 4 remaining elements to write, deal with the
   * entire quadword. Destination pointer is offset by the length of the head,
   * and offset is 0 since the previous quad was exhausted when writing the
   * head.  */
  for (; i + 4 < n; i += 4) {
    c = get_philox();
    write_elems(dst + i, c.data.u32, 4);
    advance(4);
  }

  /* Tail: if there are between 1 and 3 elements left to write. advance will
   * update offset but not bump c so that subsequent takes will use the current
   * value of c, but index into it from further on.  */
  if (i < n) {
    c = get_philox();
    const int64_t ntail = n - i;
    write_elems(dst + i, c.data.u32, ntail);
    advance(ntail);
  }
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int Philox4x3210Generator<CF>::fillBits(int64_t n, uint32_t dst[]) {
  fill_array(n, dst);
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int Philox4x3210Generator<CF>::fillBits32(int64_t n, uint32_t dst[]) {
  return fillBits(n, dst);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int Philox4x3210Generator<CF>::fillBits64(int64_t n, uint64_t dst[]) {
  return fillBits(n * 2, (uint32_t *)dst);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int Philox4x3210Generator<CF>::fill(int64_t n, float dst[]) {
  fill_array(n, dst);
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int Philox4x3210Generator<CF>::fill(int64_t n, double dst[]) {
  fill_array(n, dst);
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int Philox4x3210Generator<CF>::skipAhead(uint64_t n, const uint64_t nskip[]) {
  /* Period of Philox4x32 is 2^130 (get 4 words for each value of the 128-bit
   * counter, so all but the first 3 elements of nskip can be ignored.  */

  /* 2^0 part.  */
  advance(nskip[0]);

  /* 2^64 part - no need to mess with offset as j * 2^64 is a multiple of 4.
   * Just bump c by j * 2^62.  */
  if (n > 1)
    state.c.data.u128 += ((__uint128_t)nskip[1]) << 62;

  /* 2^128 part - we only care about first 2 bits, since any values > 4 mean
   * advancing by a multiple of 2^130, which is circular. Similar to 2^64 part,
   * advancing by j * 2^128 means bumping c by j * 2^126.  */
  if (n > 2)
    state.c.data.u128 += ((__uint128_t)nskip[2]) << 126;

  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int Philox4x3210Generator<CF>::setStride(uint64_t) {
  return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
}

} // namespace openrng
