/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/neon/sfmt19937_neon.hpp>
#include <utils/mt_skip_ahead.hpp>
#include <utils/mt_skip_ahead_data.hpp>

#include <cstring>

namespace openrng {

/**
 * Helper function that transforms values from a source, the state vector, to
 * the values buffer.
 */
template <typename T>
void transform_sfmt19937(T buffer[], uint32_t source[], const size_t length,
                         const T l, const T u) {

  if constexpr (std::is_same<T, uint32_t>::value) {
    assert(l == 0);
    assert(u == 1);
    std::memcpy(buffer, source, length * sizeof(uint32_t));
  } else {
    const T scale = u - l;
    const T norm = scale / (1ULL << 32);
    const T translation = l + scale / 2;
    size_t j = 0;
    constexpr size_t unroll = 24;
    for (; j + unroll <= length; j += unroll) {
      for (size_t k = 0; k < unroll; k += 1) {
        buffer[j + k] = translation + (T)(int32_t)source[j + k] * norm;
      }
    }

    for (; j < length; j += 1) {
      buffer[j] = translation + (T)(int32_t)source[j] * norm;
    }
  }
}

/**
 * Helper function that transforms a unit32x4_t to the desired distribution.
 */
template <typename T>
void transform_vector_sfmt19937(const uint32x4_t in, T *out, const T l,
                                const T u) {
  if constexpr (std::is_same<T, uint32_t>::value) {
    assert(l == 0);
    assert(u == 1);
    vst1q_u32(out, in);
  } else {
    //
    // Widening to double is a bit messy to do with intrinsics, but fortunately
    // we can rely on autovectorisation to handle this.
    //
    const T scale = u - l;
    const T norm = scale / (1ULL << 32);
    const T translation = l + scale / 2;
    out[0] = translation + (T)(int32_t)in[0] * norm;
    out[1] = translation + (T)(int32_t)in[1] * norm;
    out[2] = translation + (T)(int32_t)in[2] * norm;
    out[3] = translation + (T)(int32_t)in[3] * norm;
  }
}

/**
 * Helper for SFMT19937 initialization scheme.
 */
static uint32_t func1(uint32_t x) {
  return (x ^ (x >> 27)) * (uint32_t)1664525UL;
}

/**
 * Helper for SFMT19937 initialization scheme.
 */
static uint32_t func2(uint32_t x) {
  return (x ^ (x >> 27)) * (uint32_t)1566083941UL;
}

/**
 * Initialize SFMT19937 generator.
 *
 * This initialization scheme operates according to the procedure
 * described in:
 *  M. Saito and M. Matsumoto
 *  "SIMD oriented Fast Mersenne Twister (SFMT)"
 *  http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/SFMT/
 * Note that we always adopt the 'initialization by array' approach
 * regardless of whether we have multiple seed values.
 */
template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int SFMT19937Generator<CF>::initialise(const int64_t key_length,
                                       const unsigned int init_key[]) {
  int i, j, count;
  uint32_t r;
  constexpr auto lag = 11;
  constexpr auto size = N * 4;
  constexpr auto mid = (size - lag) / 2;

  // Get a pointer to uint32_t data
  auto *const data = reinterpret_cast<uint32_t *>(&state.x.data()[0]);
  memset(data, 0x8B, size * sizeof(data[0]));

  if (key_length + 1 > size) {
    count = key_length + 1;
  } else {
    count = size;
  }
  r = func1(data[0] ^ data[mid] ^ data[size - 1]);
  data[mid] += r;
  r += key_length;
  data[mid + lag] += r;
  data[0] = r;

  count--;
  for (i = 1, j = 0; (j < count) && (j < key_length); j++) {
    r = func1(data[i] ^ data[(i + mid) % size] ^ data[(i + size - 1) % size]);
    data[(i + mid) % size] += r;
    r += init_key[j] + i;
    data[(i + mid + lag) % size] += r;
    data[i] = r;
    i = (i + 1) % size;
  }
  for (; j < count; j++) {
    r = func1(data[i] ^ data[(i + mid) % size] ^ data[(i + size - 1) % size]);
    data[(i + mid) % size] += r;
    r += i;
    data[(i + mid + lag) % size] += r;
    data[i] = r;
    i = (i + 1) % size;
  }
  for (j = 0; j < size; j++) {
    r = func2(data[i] + data[(i + mid) % size] + data[(i + size - 1) % size]);
    data[(i + mid) % size] ^= r;
    r -= i;
    data[(i + mid + lag) % size] ^= r;
    data[i] = r;
    i = (i + 1) % size;
  }

  // Period certification
  constexpr uint32_t parity[4] = {0x1U, 0x0U, 0x0U, 0x13c9e684U};
  uint32_t inner = 0;
  for (i = 0; i < 4; i++) {
    inner ^= data[i] & parity[i];
  }
  for (i = 16; i > 0; i >>= 1) {
    inner ^= inner >> i;
  }
  inner &= 1;
  if (inner == 1) {
    return VSL_ERROR_OK;
  }
  for (i = 0; i < 4; i++) {
    uint32_t work = 1;
    for (j = 0; j < 32; j++) {
      if ((work & parity[i]) != 0) {
        data[i] ^= work;
        return VSL_ERROR_OK;
      }
      work = work << 1;
    }
  }
  return VSL_ERROR_OK;
}

/**
 * Refresh the state and pass the old state into TransformFunc as a uint32x4_t.
 *
 * The TransformFunc enables transforming old state while generating new state.
 * This enables better use of a CPU's vector throughput.
 */
template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
template <typename TransformFunc>
void SFMT19937Generator<CF>::refresh_state(TransformFunc transform) {
  auto x_2 = vld1q_u32((uint32_t *)&state.x[N - 2]);
  auto x_1 = vld1q_u32((uint32_t *)&state.x[N - 1]);

  auto update = [&x_1, &x_2, &transform](auto x_n_ptr, auto x_nm_ptr) {
    const auto x_n = vld1q_u8((uint8_t *)x_n_ptr);
    const auto x_nm = vld1q_u32((uint32_t *)x_nm_ptr);

    const auto x_a = veorq_u8(vextq_u8(vdupq_n_u8(0), x_n, 15), x_n);
    const auto x_b =
        vreinterpretq_u8_u32(vandq_u32(vshrq_n_u32(x_nm, 11), mask));
    const auto x_c = vextq_u8(vreinterpretq_u8_u32(x_2), vdupq_n_u8(0), 1);
    const auto x_d = vreinterpretq_u8_u32(vshlq_n_u32(x_1, 18));

    const auto x = x_a ^ x_b ^ x_c ^ x_d;
    vst1q_u8((uint8_t *)x_n_ptr, x);
    transform(vreinterpretq_u32_u8(x_n));

    x_2 = x_1;
    x_1 = vreinterpretq_u32_u8(x);
  };

  int i = 0;
  const int unroll = 2;
  for (; i <= N - M - unroll; i += unroll) {
    for (int j = 0; j < unroll; j++)
      update(&state.x[i + j], &state.x[i + j + M]);
  }
  for (; i < N - M; i++) {
    update(&state.x[i], &state.x[i + M]);
  }

  for (; i <= N - unroll; i += unroll) {
    for (int j = 0; j < unroll; j++)
      update(&state.x[i + j], &state.x[i + j + M - N]);
  }
  for (; i < N; i++) {
    update(&state.x[i], &state.x[i + M - N]);
  }
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
template <typename T>
int SFMT19937Generator<CF>::advance(int64_t n, T buffer[], const T l,
                                    const T u) {

  auto state_u32 = (uint32_t *)state.x.data();

  int i = 0;
  while (i != n) {
    if (state.current_idx == N * 4) {
      // TransformFunc is a nop, the state has already been used.
      refresh_state([](uint32x4_t) {});
      state.current_idx = 0;
    }

    // Copy up until we need to do a state update
    const size_t copy_length = std::min(N * 4 - state.current_idx, n - i);

    if (copy_length == N * 4) {
      //
      // If we're consuming a whole buffer of state, we can refresh while
      // transforming.
      //
      auto curr_buffer = buffer + i;
      refresh_state([l, u, &curr_buffer](uint32x4_t in) {
        transform_vector_sfmt19937(in, curr_buffer, l, u);
        curr_buffer += 4;
      });
    } else {
      transform_sfmt19937(&buffer[i], &state_u32[state.current_idx],
                          copy_length, l, u);
      state.current_idx += copy_length;
    }
    i += copy_length;
  }

  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int SFMT19937Generator<CF>::fillBits(int64_t n, uint32_t buffer[]) {
  return advance(n, buffer, (uint32_t)0, (uint32_t)1);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int SFMT19937Generator<CF>::fill(int64_t n, float buffer[]) {
  return fill(n, buffer, 0.0, 1.0);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int SFMT19937Generator<CF>::fill(int64_t n, double buffer[]) {
  return fill(n, buffer, 0.0, 1.0);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int SFMT19937Generator<CF>::fill(int64_t n, float buffer[], const float l,
                                 const float u) {
  static_assert(sizeof(float) == sizeof(uint32_t));
  advance(n, buffer, l, u);
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int SFMT19937Generator<CF>::fill(int64_t n, double buffer[], const double l,
                                 const double u) {
  advance(n, buffer, l, u);
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int SFMT19937Generator<CF>::setStride(uint64_t) {
  return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
void SFMT19937Generator<CF>::advanceOnceInPlace(
    ring_buffer<poly128_t, SFMT19937StateNeon::N> &x) {

  auto x_2 = vld1q_u32((uint32_t *)&x[2]);
  auto x_1 = vld1q_u32((uint32_t *)&x[1]);

  const auto x_n = vld1q_u8((uint8_t *)&x[N]);
  const auto x_nm = vld1q_u32((uint32_t *)&x[N - M]);

  const auto x_a = veorq_u8(vextq_u8(vdupq_n_u8(0), x_n, 15), x_n);
  const auto x_b = vreinterpretq_u8_u32(vandq_u32(vshrq_n_u32(x_nm, 11), mask));
  const auto x_c = vextq_u8(vreinterpretq_u8_u32(x_2), vdupq_n_u8(0), 1);
  const auto x_d = vreinterpretq_u8_u32(vshlq_n_u32(x_1, 18));

  poly128_t x_0;
  vst1q_u8((uint8_t *)&x_0, x_a ^ x_b ^ x_c ^ x_d);

  x.push_back(x_0);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int SFMT19937Generator<CF>::skipAhead(uint64_t n, const uint64_t nskip[]) {
  if (n > 1) {
    return VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED;
  }

  const auto newIdx = 1 + ((state.current_idx + nskip[0] - 1) % 4);
  const auto nStateUpdates = (state.current_idx + nskip[0] - 1) / 4;

  mt_skip_ahead<poly128_t, N>(state.x, &advanceOnceInPlace, nStateUpdates,
                              skipahead::data::sfmt19937_cache);

  state.current_idx = newIdx;

  return VSL_ERROR_OK;
}

} // namespace openrng
