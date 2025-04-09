/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/quad_counter_based_generator.hpp>
#include <utils/neon_intrinsics.hpp>

#include <arm_neon.h>

namespace openrng {

namespace {

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

[[maybe_unused]] void st4(uint32_t *dst, uint32x4x4_t x) { vst4q_u32(dst, x); }

[[maybe_unused]] void st4(float *dst, uint32x4x4_t x) {
  vst4q_f32(dst, transform_to_float(x));
}

[[maybe_unused]] void st4(double *dst, uint32x4x4_t x) {
  const float64x2_t delta = vdupq_n_f64(0.5);
  const float64x2_t scale = vdupq_n_f64(0x1p-32);

  const int32x4x4_t signed_x{
      vreinterpretq_s32_u32(x.val[0]), vreinterpretq_s32_u32(x.val[1]),
      vreinterpretq_s32_u32(x.val[2]), vreinterpretq_s32_u32(x.val[3])};

  float64x2_t val_0_l = vcvtq_f64_s64(vmovl_s32(vget_low_s32(signed_x.val[0])));
  float64x2_t val_0_h =
      vcvtq_f64_s64(vmovl_s32(vget_high_s32(signed_x.val[0])));

  float64x2_t val_1_l = vcvtq_f64_s64(vmovl_s32(vget_low_s32(signed_x.val[1])));
  float64x2_t val_1_h =
      vcvtq_f64_s64(vmovl_s32(vget_high_s32(signed_x.val[1])));

  float64x2_t val_2_l = vcvtq_f64_s64(vmovl_s32(vget_low_s32(signed_x.val[2])));
  float64x2_t val_2_h =
      vcvtq_f64_s64(vmovl_s32(vget_high_s32(signed_x.val[2])));

  float64x2_t val_3_l = vcvtq_f64_s64(vmovl_s32(vget_low_s32(signed_x.val[3])));
  float64x2_t val_3_h =
      vcvtq_f64_s64(vmovl_s32(vget_high_s32(signed_x.val[3])));

  val_0_l = vfmaq_f64(delta, val_0_l, scale);
  val_0_h = vfmaq_f64(delta, val_0_h, scale);

  val_1_l = vfmaq_f64(delta, val_1_l, scale);
  val_1_h = vfmaq_f64(delta, val_1_h, scale);

  val_2_l = vfmaq_f64(delta, val_2_l, scale);
  val_2_h = vfmaq_f64(delta, val_2_h, scale);

  val_3_l = vfmaq_f64(delta, val_3_l, scale);
  val_3_h = vfmaq_f64(delta, val_3_h, scale);

  const float64x2x4_t lows = {val_0_l, val_1_l, val_2_l, val_3_l};
  const float64x2x4_t highs = {val_0_h, val_1_h, val_2_h, val_3_h};

  vst4q_f64(dst, lows);
  vst4q_f64(dst + 8, highs);
}

} // namespace

template <CPUFeatures CF, typename Key>
  requires(NeonOrSha3<CF>)
class QuadCounterBasedGenerator<CF, Key>
    : public CopyableGenerator<QuadCounterBasedState<Key>> {

private:
  inline void advance(uint64_t by) {
    // Increment c every 4 advances.
    this->state.c += (this->state.off + by) / 4;
    // Adjust offset of the next subelement to be outputted.
    this->state.off = (this->state.off + by) % 4;
  }

protected:
  template <
      typename T, uint128_t (*do_bijection)(const QuadCounterBasedState<Key> &),
      void (*do_bijection_12)(const QuadCounterBasedState<Key> &,
                              uint32x4x4_t &, uint32x4x4_t &, uint32x4x4_t &)>
  void fill_array(int64_t n, T *dst) {
    int64_t i = 0;

    // Exhaust remaining elements
    uint128_t c = do_bijection(this->state);
    const int64_t nhead =
        std::min(static_cast<int64_t>(4) - this->state.off, n);
    write_elems(dst, c.data.u32 + this->state.off, nhead);
    advance(nhead);
    i += nhead;

    // Unrolled body - unroll by a factor of 12.
    for (; i + 48 < n; i += 48) {
      uint128_t x[12];
      for (int j = 0; j < 12; j++)
        x[j] = this->state.c + j;

      uint32x4x4_t x0 = vld4q_u32(x[0].data.u32), x1 = vld4q_u32(x[4].data.u32),
                   x2 = vld4q_u32(x[8].data.u32);
      do_bijection_12(this->state, x0, x1, x2);
      st4(dst + i, x0);
      st4(dst + i + 16, x1);
      st4(dst + i + 32, x2);
      advance(48);
    }

    // Process 4 elements as a quadword at a time
    for (; i + 4 < n; i += 4) {
      c = do_bijection(this->state);
      write_elems(dst + i, c.data.u32, 4);
      advance(4);
    }

    // Handle remaining elements left
    if (i < n) {
      c = do_bijection(this->state);
      const int64_t ntail = n - i;
      write_elems(dst + i, c.data.u32, ntail);
      advance(ntail);
    }
  }

public:
  QuadCounterBasedGenerator(int64_t _brngId)
      : CopyableGenerator<QuadCounterBasedState<Key>>{_brngId} {}

  int setStride(uint64_t) final { return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED; }

  int skipAhead(uint64_t n, const uint64_t nskip[]) final {
    // Period is 2^130, so all but the first 3 elements of nskip can be ignored.

    // 2^0 part
    advance(nskip[0]);

    // 2^64 part - no need to mess with offset as j * 2^64 is a multiple of 4.
    if (n > 1)
      this->state.c += uint128_t{nskip[1]} << 62;

    // 2^128 part - we only care about first 2 bits, since any values > 4 mean
    // advancing by a multiple of 2^130, which is circular.
    if (n > 2)
      this->state.c += uint128_t{nskip[2]} << 126;

    return VSL_ERROR_OK;
  }
};

} // namespace openrng
