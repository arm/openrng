/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/mt2203_params.hpp>
#include <generator/mt_initialization.hpp>
#include <generator/neon/mt2203_neon.hpp>

#include <arm_neon.h>
#include <cstring>

namespace openrng {

/**
 * Initialize MT2203 generator.
 *
 * This initialization scheme operates according to the procedure
 * described in:
 *  M. Matsumoto and T. Nishimura,
 *  "Mersenne Twister with improved initialization"
 *  http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/MT2002/emt2203ar.html
 * Note that we always adopt the 'initialization by array' approach
 * regardless of whether we have multiple seed values.
 */
template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT2203Generator<CF>::initialise(const int64_t n,
                                    const unsigned int params[]) {
  state.a = MT2203_PARAMS[brngId - VSL_BRNG_MT2203].a;
  /* Invert b and c so they can be used with BCAX.  */
  state.b = ~MT2203_PARAMS[brngId - VSL_BRNG_MT2203].b;
  state.c = ~MT2203_PARAMS[brngId - VSL_BRNG_MT2203].c;
  return mt_initialise_by_array<N>(n, params, state.x.data());
}

namespace {
template <typename T>
void update(uint32_t *x_0, uint32_t *x_1, uint32_t *x_nm, uint32_t a,
            uint32_t b, uint32_t c, T *buf, T l, T u) {
  auto x = (*x_0 & 0xffffffe0) | (*x_1 & 0x1f);
  x = *x_nm ^ ((x & 0x1) ? (x >> 1) ^ a : x >> 1);

  // Update x buffer
  *x_0 = x;

  uint32_t y = x;
  y ^= y >> 12;
  y ^= (y << 7) & ~b;
  y ^= (y << 15) & ~c;
  y ^= y >> 18;

  if constexpr (std::is_same_v<T, uint32_t>) {
    /* If we are in the uint32_t version, we are either doing a uint32_t
       distribution or this is the tail portion of the refresh loop, where the
       tempered output has to be stored to state.tempered and then transformed
       separately (could still be here in the uint32_t version as well).  */
    *buf = y;
  } else {
    const T scale = u - l;
    /* We are doing an in-place floating-point refresh.  */
    *buf = ((T)y / (1ULL << 32)) * scale + l;
  }
}

template <CPUFeatures CF, typename T>
void update_16(uint32_t *x_0_ptr, uint32_t *x_1_ptr, uint32_t *x_nm_ptr,
               uint32_t a, uint32_t b, uint32_t c, T *buf, T l, T u) {
  const uint32x4_t x_0_0 = vld1q_u32(x_0_ptr);
  const uint32x4_t x_0_1 = vld1q_u32(x_0_ptr + 4);
  const uint32x4_t x_0_2 = vld1q_u32(x_0_ptr + 8);
  const uint32x4_t x_0_3 = vld1q_u32(x_0_ptr + 12);

  const uint32x4_t x_1_0 = vld1q_u32(x_1_ptr);
  const uint32x4_t x_1_1 = vld1q_u32(x_1_ptr + 4);
  const uint32x4_t x_1_2 = vld1q_u32(x_1_ptr + 8);
  const uint32x4_t x_1_3 = vld1q_u32(x_1_ptr + 12);

  const uint32x4_t x_nm_0 = vld1q_u32(x_nm_ptr);
  const uint32x4_t x_nm_1 = vld1q_u32(x_nm_ptr + 4);
  const uint32x4_t x_nm_2 = vld1q_u32(x_nm_ptr + 8);
  const uint32x4_t x_nm_3 = vld1q_u32(x_nm_ptr + 12);

  uint32x4_t x_0 = vbslq_u32(vdupq_n_u32(0xffffffe0), x_0_0, x_1_0);
  uint32x4_t x_1 = vbslq_u32(vdupq_n_u32(0xffffffe0), x_0_1, x_1_1);
  uint32x4_t x_2 = vbslq_u32(vdupq_n_u32(0xffffffe0), x_0_2, x_1_2);
  uint32x4_t x_3 = vbslq_u32(vdupq_n_u32(0xffffffe0), x_0_3, x_1_3);

  uint32x4_t arg_0 = vshrq_n_u32(x_0, 1);
  uint32x4_t arg_1 = vshrq_n_u32(x_1, 1);
  uint32x4_t arg_2 = vshrq_n_u32(x_2, 1);
  uint32x4_t arg_3 = vshrq_n_u32(x_3, 1);

  const uint32x4_t arg_xor_0 = veorq_u32(arg_0, vdupq_n_u32(a));
  const uint32x4_t arg_xor_1 = veorq_u32(arg_1, vdupq_n_u32(a));
  const uint32x4_t arg_xor_2 = veorq_u32(arg_2, vdupq_n_u32(a));
  const uint32x4_t arg_xor_3 = veorq_u32(arg_3, vdupq_n_u32(a));

  const uint32x4_t cmp_0 = vtstq_u32(x_1_0, vdupq_n_u32(0x1));
  const uint32x4_t cmp_1 = vtstq_u32(x_1_1, vdupq_n_u32(0x1));
  const uint32x4_t cmp_2 = vtstq_u32(x_1_2, vdupq_n_u32(0x1));
  const uint32x4_t cmp_3 = vtstq_u32(x_1_3, vdupq_n_u32(0x1));

  arg_0 = vbslq_u32(cmp_0, arg_xor_0, arg_0);
  arg_1 = vbslq_u32(cmp_1, arg_xor_1, arg_1);
  arg_2 = vbslq_u32(cmp_2, arg_xor_2, arg_2);
  arg_3 = vbslq_u32(cmp_3, arg_xor_3, arg_3);

  x_0 = veorq_u32(x_nm_0, arg_0);
  x_1 = veorq_u32(x_nm_1, arg_1);
  x_2 = veorq_u32(x_nm_2, arg_2);
  x_3 = veorq_u32(x_nm_3, arg_3);

  vst1q_u32(x_0_ptr + 0, x_0);
  vst1q_u32(x_0_ptr + 4, x_1);
  vst1q_u32(x_0_ptr + 8, x_2);
  vst1q_u32(x_0_ptr + 12, x_3);

  uint32x4_t y_0 = x_0;
  uint32x4_t y_1 = x_1;
  uint32x4_t y_2 = x_2;
  uint32x4_t y_3 = x_3;

  y_0 = veorq_u32(y_0, vshrq_n_u32(y_0, 12));
  y_1 = veorq_u32(y_1, vshrq_n_u32(y_1, 12));
  y_2 = veorq_u32(y_2, vshrq_n_u32(y_2, 12));
  y_3 = veorq_u32(y_3, vshrq_n_u32(y_3, 12));

  if constexpr (CF == CPUFeatures::neon) {
    y_0 = veorq_u32(y_0, vbicq_u32(vshlq_n_u32(y_0, 7), vdupq_n_u32(b)));
    y_1 = veorq_u32(y_1, vbicq_u32(vshlq_n_u32(y_1, 7), vdupq_n_u32(b)));
    y_2 = veorq_u32(y_2, vbicq_u32(vshlq_n_u32(y_2, 7), vdupq_n_u32(b)));
    y_3 = veorq_u32(y_3, vbicq_u32(vshlq_n_u32(y_3, 7), vdupq_n_u32(b)));

    y_0 = veorq_u32(y_0, vbicq_u32(vshlq_n_u32(y_0, 15), vdupq_n_u32(c)));
    y_1 = veorq_u32(y_1, vbicq_u32(vshlq_n_u32(y_1, 15), vdupq_n_u32(c)));
    y_2 = veorq_u32(y_2, vbicq_u32(vshlq_n_u32(y_2, 15), vdupq_n_u32(c)));
    y_3 = veorq_u32(y_3, vbicq_u32(vshlq_n_u32(y_3, 15), vdupq_n_u32(c)));
  } else {
    y_0 = vbcaxq_u32(y_0, vshlq_n_u32(y_0, 7), vdupq_n_u32(b));
    y_1 = vbcaxq_u32(y_1, vshlq_n_u32(y_1, 7), vdupq_n_u32(b));
    y_2 = vbcaxq_u32(y_2, vshlq_n_u32(y_2, 7), vdupq_n_u32(b));
    y_3 = vbcaxq_u32(y_3, vshlq_n_u32(y_3, 7), vdupq_n_u32(b));

    y_0 = vbcaxq_u32(y_0, vshlq_n_u32(y_0, 15), vdupq_n_u32(c));
    y_1 = vbcaxq_u32(y_1, vshlq_n_u32(y_1, 15), vdupq_n_u32(c));
    y_2 = vbcaxq_u32(y_2, vshlq_n_u32(y_2, 15), vdupq_n_u32(c));
    y_3 = vbcaxq_u32(y_3, vshlq_n_u32(y_3, 15), vdupq_n_u32(c));
  }

  y_0 = veorq_u32(y_0, vshrq_n_u32(y_0, 18));
  y_1 = veorq_u32(y_1, vshrq_n_u32(y_1, 18));
  y_2 = veorq_u32(y_2, vshrq_n_u32(y_2, 18));
  y_3 = veorq_u32(y_3, vshrq_n_u32(y_3, 18));

  if constexpr (std::is_same_v<T, uint32_t>) {
    vst1q_u32(buf + 0, y_0);
    vst1q_u32(buf + 4, y_1);
    vst1q_u32(buf + 8, y_2);
    vst1q_u32(buf + 12, y_3);
  } else if constexpr (std::is_same_v<T, float>) {
    float32x4_t z_0 = vcvtq_f32_u32(y_0), z_1 = vcvtq_f32_u32(y_1),
                z_2 = vcvtq_f32_u32(y_2), z_3 = vcvtq_f32_u32(y_3);
    const float32x4_t scale = vdupq_n_f32((u - l) * 0x1p-32);
    const float32x4_t lv = vdupq_n_f32(l);
    z_0 = vfmaq_f32(lv, z_0, scale);
    z_1 = vfmaq_f32(lv, z_1, scale);
    z_2 = vfmaq_f32(lv, z_2, scale);
    z_3 = vfmaq_f32(lv, z_3, scale);

    vst1q_f32(buf + 0, z_0);
    vst1q_f32(buf + 4, z_1);
    vst1q_f32(buf + 8, z_2);
    vst1q_f32(buf + 12, z_3);
  } else if constexpr (std::is_same_v<T, double>) {
    const float64x2_t scale = vdupq_n_f64((u - l) * 0x1p-32);
    const float64x2_t lv = vdupq_n_f64(l);

    float64x2_t val_0_l = vcvtq_f64_u64(vmovl_u32(vget_low_u32(y_0)));
    float64x2_t val_0_h = vcvtq_f64_u64(vmovl_u32(vget_high_u32(y_0)));

    float64x2_t val_1_l = vcvtq_f64_u64(vmovl_u32(vget_low_u32(y_1)));
    float64x2_t val_1_h = vcvtq_f64_u64(vmovl_u32(vget_high_u32(y_1)));

    float64x2_t val_2_l = vcvtq_f64_u64(vmovl_u32(vget_low_u32(y_2)));
    float64x2_t val_2_h = vcvtq_f64_u64(vmovl_u32(vget_high_u32(y_2)));

    float64x2_t val_3_l = vcvtq_f64_u64(vmovl_u32(vget_low_u32(y_3)));
    float64x2_t val_3_h = vcvtq_f64_u64(vmovl_u32(vget_high_u32(y_3)));

    val_0_l = vfmaq_f64(lv, val_0_l, scale);
    val_0_h = vfmaq_f64(lv, val_0_h, scale);

    val_1_l = vfmaq_f64(lv, val_1_l, scale);
    val_1_h = vfmaq_f64(lv, val_1_h, scale);

    val_2_l = vfmaq_f64(lv, val_2_l, scale);
    val_2_h = vfmaq_f64(lv, val_2_h, scale);

    val_3_l = vfmaq_f64(lv, val_3_l, scale);
    val_3_h = vfmaq_f64(lv, val_3_h, scale);

    vst1q_f64(buf + 0, val_0_l);
    vst1q_f64(buf + 2, val_0_h);
    vst1q_f64(buf + 4, val_1_l);
    vst1q_f64(buf + 6, val_1_h);
    vst1q_f64(buf + 8, val_2_l);
    vst1q_f64(buf + 10, val_2_h);
    vst1q_f64(buf + 12, val_3_l);
    vst1q_f64(buf + 14, val_3_h);
  }
}

} // namespace

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
template <typename T>
void MT2203Generator<CF>::refresh_state(T buf[], T l, T u) {
  if constexpr (std::is_same_v<T, uint32_t>) {
    assert(l == 0);
    assert(u == 1);
  }

  int i = 0;
  for (; i + 16 <= N - M; i += 16) {
    update_16<CF>(&state.x[i], &state.x[i + 1], &state.x[i + M], state.a,
                  state.b, state.c, &buf[i], l, u);
  }
  for (; i < N - M; i++) {
    update(&state.x[i], &state.x[i + 1], &state.x[i + M], state.a, state.b,
           state.c, &buf[i], l, u);
  }
  for (; i + 16 <= N - 1; i += 16) {
    update_16<CF>(&state.x[i], &state.x[i + 1], &state.x[i + M - N], state.a,
                  state.b, state.c, &buf[i], l, u);
  }
  for (; i < N - 1; i++) {
    update(&state.x[i], &state.x[i + 1], &state.x[i + M - N], state.a, state.b,
           state.c, &buf[i], l, u);
  }
  update(&state.x[i], &state.x[0], &state.x[M - 1], state.a, state.b, state.c,
         &buf[i], l, u);
}

namespace {

void transform_mt2203(uint32_t buffer[], uint32_t source[], const size_t length,
                      uint32_t, uint32_t) {
  std::memcpy(buffer, source, length * sizeof(uint32_t));
}

template <std::floating_point T>
void transform_mt2203(T buffer[], uint32_t source[], const size_t length, T l,
                      T u) {
  const T scale = u - l;
  const T norm = scale * (T)(0x1p-32);
  for (size_t j = 0; j < length; j += 1) {
    buffer[j] = ((T)source[j] * norm) + l;
  }
}
} // namespace

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
template <typename T>
uint32_t MT2203Generator<CF>::advance(int64_t n, T buffer[], T l, T u) {
  int i = 0;
  while (i != n) {
    // Copy up until we need to do a state update
    const size_t copy_length = std::min(static_cast<size_t>(N - state.index),
                                        static_cast<size_t>(n - i));

    if (copy_length == N) {
      /* If copy length is the entire state, do the refresh directly into the
         buffer rather than using the intermediate cache.  */
      assert(state.index == 0);
      refresh_state(&buffer[i], l, u);
    } else {
      /* This is either the head (previous sample left us with a nonzero
         index) or tail (less than N elements remaining to copy after a
         refresh).  */
      if (state.index == 0) {
        /* Refresh into the cache, rather than the buffer, as we will need
           some of these values again next sample.  */
        refresh_state(&state.tempered[0], 0u, 1u);
      }
      /* Copy from the cache into buffer.  */
      transform_mt2203(&buffer[i], &state.tempered[state.index], copy_length, l,
                       u);
    }

    state.index = (state.index + copy_length) % N;
    i += copy_length;
  }
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT2203Generator<CF>::fillBits(int64_t n, uint32_t buffer[]) {
  return advance(n, buffer, 0u, 1u);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT2203Generator<CF>::fill(int64_t n, float buffer[]) {
  return fill(n, buffer, 0.f, 1.f);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT2203Generator<CF>::fill(int64_t n, double buffer[]) {
  return fill(n, buffer, 0., 1.);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT2203Generator<CF>::fill(int64_t n, float buffer[], const float l,
                              const float u) {
  static_assert(sizeof(float) == sizeof(uint32_t));
  advance(n, buffer, l, u);
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT2203Generator<CF>::fill(int64_t n, double buffer[], const double l,
                              const double u) {
  advance(n, buffer, l, u);
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT2203Generator<CF>::setStride(uint64_t) {
  return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT2203Generator<CF>::skipAhead(uint64_t, const uint64_t[]) {
  return VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED;
}
} // namespace openrng
