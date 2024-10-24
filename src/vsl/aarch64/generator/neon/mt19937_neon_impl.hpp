/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/neon/mt19937_neon.hpp>
#include <utils/dynamic_array.hpp>
#include <utils/mt_skip_ahead.hpp>
#include <utils/mt_skip_ahead_data.hpp>
#include <utils/neon_intrinsics.hpp>

#include <algorithm>
#include <arm_neon.h>
#include <cstring>

namespace openrng {

/**
 * Helper function to transfer values from a
 * source, e.g. the state vector, to the
 * values buffer, with the appropriate transformation,
 * if required.
 */
template <typename T>
void transform_mt19937(T buffer[], uint32_t source[], const size_t length,
                       const T l, const T u) {

  if constexpr (std::is_same<T, uint32_t>::value) {
    assert(l == 0);
    assert(u == 1);
    std::memcpy(buffer, source, length * sizeof(uint32_t));
  } else {
    constexpr T norm = 1.f / (T)(1ULL << 32);
    const T scale = u - l;
    size_t j = 0;
    constexpr size_t unroll = 24;
    for (; j + unroll <= length; j += unroll) {
      for (size_t k = 0; k < unroll; k += 1) {
        buffer[j + k] = ((T)source[j + k] * norm) * scale + l;
      }
    }

    for (; j < length; j += 1) {
      buffer[j] = ((T)source[j] * norm) * scale + l;
    }
  }
}

/**
 * Initialize MT19937 generator.
 *
 * This initialization scheme operates according to the procedure
 * described in:
 *  M. Matsumoto and T. Nishimura,
 *  "Mersenne Twister with improved initialization"
 *  http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/MT2002/emt19937ar.html
 * Note that we always adopt the 'initialization by array' approach
 * regardless of whether we have multiple seed values.
 */
template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT19937Generator<CF>::initialise(const int64_t n,
                                     const unsigned int params[]) {
  const auto data = state.x.data();
  data[0] = 19650218;
  for (int i = 1; i < N; i++) {
    data[i] = 1812433253ULL * (data[i - 1] ^ (data[i - 1] >> 30)) + i;
  }

  int i = 1;
  for (int j = 0, k = 0; k < std::max(n, N); k++) {
    data[i] ^= 1664525ULL * (data[i - 1] ^ (data[i - 1] >> 30));
    data[i] += params[j] + j;
    i = i == N - 1 ? 1 : i + 1;
    j = j == n - 1 ? 0 : j + 1;
    if (i == 1) {
      data[0] = data[N - 1];
    }
  }

  for (int k = 0; k < N - 1; k++) {
    data[i] ^= 1566083941ULL * (data[i - 1] ^ (data[i - 1] >> 30));
    data[i] -= i;
    i = i == N - 1 ? 1 : i + 1;
    if (i == 1) {
      data[0] = data[N - 1];
    }
  }

  data[0] = 1 << 31;
  return VSL_ERROR_OK;
}

/**
 * Perform a state update on `NumElems` consecutive elements.
 *
 * @tparam NumElems    Number of consecutive elements to update.
 * @param[in,out] x_0  Pointer to x_0. Result is written here.
 * @param[in]     x_1  Pointer to x_1.
 * @param[in]     x_nm Pointer to x_(n-m).
 */
template <int NumElems> inline void update(uint32_t *, uint32_t *, uint32_t *);

/**
 * Scalar update.
 */
template <>
inline void update<1>(uint32_t *x_0, uint32_t *x_1, uint32_t *x_nm) {
  auto x = (*x_0 & 0x80000000) | (*x_1 & 0x7FFFFFFF);
  x = *x_nm ^ ((x & 0x1) ? (x >> 1) ^ 0x9908B0DF : x >> 1);
  *x_0 = x;
}

/**
 * 4-element (one vector) update.
 */
template <>
inline void update<4>(uint32_t *x_0_ptr, uint32_t *x_1_ptr,
                      uint32_t *x_nm_ptr) {
  const uint32x4_t x_0 = vld1q_u32(x_0_ptr);
  const uint32x4_t x_1 = vld1q_u32(x_1_ptr);
  const uint32x4_t x_nm = vld1q_u32(x_nm_ptr);

  uint32x4_t x = vbslq_u32(vdupq_n_u32(0x80000000), x_0, x_1);

  uint32x4_t arg = vshrq_n_u32(x, 1);
  const uint32x4_t arg_xor = veorq_u32(arg, vdupq_n_u32(0x9908B0DF));

  // TODO: maybe find a nicer way to do this
  const uint32x4_t cmp = vtstq_u32(x, vdupq_n_u32(0x1));
  arg = vbslq_u32(cmp, arg_xor, arg);
  x = veorq_u32(x_nm, arg);

  vst1q_u32(x_0_ptr, x);
}

/**
 * Refresh all `N` elements of state buffer.
 *
 * Separated into four distinct updates to allow for easier indexing
 * and vectorisation:
 *
 * Update 1: elements 0 to `N - M - 4` (vectorized)
 * Update 2: elements `N - M - 3` to `N - M` (scalar)
 * Update 3: elements `N - M` to `N - 2` (vectorized)
 * Update 4: element `N - 1` (scalar)
 */
template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
void MT19937Generator<CF>::refresh_state() {
  int i = 0;

  // Step 1: compute elements of x

  // Update elements 0 to 223 (56 iterations)
  // TODO: write & try out update<8>
  for (; i < N - M - 3; i += 4) {
    update<4>(&state.x[i], &state.x[i + 1], &state.x[i + M]);
  }

  // Update elements 224, 225, 226 (3 iterations)
  // TODO: write & use update<2> here!
  for (; i < N - M; i += 1) {
    update<1>(&state.x[i], &state.x[i + 1], &state.x[i + M]);
  }

  // Update elements 227 to 622 (99 iterations)
  for (; i < N - 1; i += 4) {
    update<4>(&state.x[i], &state.x[i + 1], &state.x[i + M - N]);
  }

  // Update final element (623)
  update<1>(&state.x[i], &state.x[0], &state.x[M - 1]);

  /* K1 and K2 marked volatile to prevent them being folded, which would prevent
     BCAX from being emitted for the SHA3 version.  */
  const volatile uint32x4_t K1 = vdupq_n_u32(~0x9D2C5680),
                            K2 = vdupq_n_u32(~0xEFC60000);

  // Step 2: compute next elements of y
  for (i = 0; i < N; i += 16) {
    uint32x4x4_t y = vld1q_u32_x4(&state.x[i]);
    for (int j = 0; j < 4; j++) {
      y.val[j] = veorq_u32(y.val[j], vshrq_n_u32(y.val[j], 11));
      y.val[j] = veorq_u32(y.val[j], vbicq_u32(vshlq_n_u32(y.val[j], 7), K1));
      y.val[j] = veorq_u32(y.val[j], vbicq_u32(vshlq_n_u32(y.val[j], 15), K2));
      y.val[j] = veorq_u32(y.val[j], vshrq_n_u32(y.val[j], 18));
    }
    vst1q_u32_x4(&state.y[i], y);
  }
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
template <typename T>
uint32_t MT19937Generator<CF>::advance(int64_t n, T buffer[], const T l,
                                       const T u) {
  int i = 0;
  while (i != n) {
    if (state.index == N) {
      // TODO: here we should be able to write into the
      //       user's buffer and skip the memcpy below
      refresh_state();
      state.index = 0;
    }
    // Copy up until we need to do a state update
    const size_t copy_length = std::min(static_cast<size_t>(N - state.index),
                                        static_cast<size_t>(n - i));

    transform_mt19937(&buffer[i], &state.y[state.index], copy_length, l, u);

    state.index += copy_length;
    i += copy_length;
  }
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT19937Generator<CF>::fillBits(int64_t n, uint32_t buffer[]) {
  return advance(n, buffer, 0u, 1u);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT19937Generator<CF>::fillBits32(int64_t n, uint32_t buffer[]) {
  // MT19937 already generates uniformly distributed integers in [0, 2^32]
  return fillBits(n, buffer);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT19937Generator<CF>::fillBits64(int64_t n, uint64_t buffer[]) {
  return fillBits32(2 * n, reinterpret_cast<uint32_t *>(buffer));
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT19937Generator<CF>::fill(int64_t n, float buffer[]) {
  return fill(n, buffer, 0.0, 1.0);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT19937Generator<CF>::fill(int64_t n, double buffer[]) {
  return fill(n, buffer, 0.0, 1.0);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT19937Generator<CF>::fill(int64_t n, float buffer[], const float l,
                               const float u) {
  static_assert(sizeof(float) == sizeof(uint32_t));
  advance(n, buffer, l, u);
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT19937Generator<CF>::fill(int64_t n, double buffer[], const double l,
                               const double u) {
  advance(n, buffer, l, u);
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT19937Generator<CF>::setStride(uint64_t) {
  return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
void MT19937Generator<CF>::advanceOnceInPlace(
    std::array<uint32_t, MT19937State::N> &x) {
  auto tmp = (x[0] & 0x80000000) | (x[1] & 0x7FFFFFFF);
  x[0] = x[M] ^ ((tmp & 0x1) ? (tmp >> 1) ^ a : tmp >> 1);
  std::rotate(x.begin(), x.begin() + 1, x.end());
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
void xorStatesInPlace(std::array<uint32_t, MT19937State::N> &result,
                      const std::array<uint32_t, MT19937State::N> &other) {
  for (size_t i = 0; i < MT19937State::N; i++) {
    result[i] ^= other[i];
  }
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int MT19937Generator<CF>::skipAhead(uint64_t n, const uint64_t nskip[]) {
  if (n > 1) {
    return VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED;
  }

  auto skip = nskip[0];

  DynamicArray<const DynamicArray<uint64_t> *> q(64);
  int howmany_nnz = 0;
  for (int j = 0; skip; ++j, skip >>= 1) {
    if ((skip & 1) != 0) {
      q[howmany_nnz] = &skipahead::data::mt19937_cache[j];
      howmany_nnz += 1;
    }
  }

  for (int hm = 0; hm < howmany_nnz; ++hm) {
    auto stateCopy = state.x;
    decltype(state.x) resultState = {};
    auto coefficient = *q[hm];
    for (size_t c_bit = 0; c_bit < coefficient.size(); ++c_bit) {
      const uint64_t b = coefficient[c_bit];
      for (int i = 0; i < 64; ++i) {
        if (1 & b >> i) {
          xorStatesInPlace<CF>(resultState, stateCopy);
        }
        advanceOnceInPlace(stateCopy);
      }
    }
    state.x = resultState;
  }

  //
  // We need to recompute any cached state.
  //
  for (int i = 0; i < N; i++) {
    auto y = state.x[i];
    y ^= y >> 11;
    y ^= (y << 7) & 0x9D2C5680;
    y ^= (y << 15) & 0xEFC60000;
    y ^= y >> 18;
    state.y[i] = y;
  }

  return VSL_ERROR_OK;
}

} // namespace openrng
