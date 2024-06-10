/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/neon/sfmt19937_neon.hpp>
#include <utils/mt_skip_ahead.hpp>

#include <cstring>

namespace openrng {

/**
 * Helper function that to transfer values from
 * a source, the state vector, to the values buffer
 */
template <typename T>
void transform_sfmt19937(T buffer[], uint32_t source[], const size_t length,
                         const T l, const T u) {

  if constexpr (std::is_same<T, uint32_t>::value) {
    assert(l == 0);
    assert(u == 1);
    std::memcpy(buffer, source, length * sizeof(uint32_t));
  } else {
    constexpr T norm = (T)1.0 / (T)(1ULL << 32);
    constexpr T translation = (T)0.5;
    const T scale = u - l;
    size_t j = 0;
    constexpr size_t unroll = 24;
    for (; j + unroll <= length; j += unroll) {
      for (size_t k = 0; k < unroll; k += 1) {
        buffer[j + k] =
            (translation + (T)(int32_t)source[j + k] * norm) * scale + l;
      }
    }

    for (; j < length; j += 1) {
      buffer[j] = (translation + (T)(int32_t)source[j] * norm) * scale + l;
    }
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

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
void SFMT19937Generator<CF>::refresh_state() {
  auto x_2 = vld1q_u32((uint32_t *)&state.x[N - 2]);
  auto x_1 = vld1q_u32((uint32_t *)&state.x[N - 1]);

  auto update = [&x_1, &x_2](auto x_n_ptr, auto x_nm_ptr) {
    const auto x_n = vld1q_u8((uint8_t *)x_n_ptr);
    const auto x_nm = vld1q_u32((uint32_t *)x_nm_ptr);

    const auto x_a = veorq_u8(vextq_u8(vdupq_n_u8(0), x_n, 15), x_n);
    const auto x_b =
        vreinterpretq_u8_u32(vandq_u32(vshrq_n_u32(x_nm, 11), mask));
    const auto x_c = vextq_u8(vreinterpretq_u8_u32(x_2), vdupq_n_u8(0), 1);
    const auto x_d = vreinterpretq_u8_u32(vshlq_n_u32(x_1, 18));

    const auto x = x_a ^ x_b ^ x_c ^ x_d;
    vst1q_u8((uint8_t *)x_n_ptr, x);

    x_2 = x_1;
    x_1 = vreinterpretq_u32_u8(x);
  };

  int i = 0;
  for (; i < N - M; i++) {
    update(&state.x[i], &state.x[i + M]);
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
      refresh_state();
      state.current_idx = 0;
    }
    // Copy up until we need to do a state update
    const size_t copy_length = std::min(N * 4 - state.current_idx, n - i);

    transform_sfmt19937(&buffer[i], &state_u32[state.current_idx], copy_length,
                        l, u);

    state.current_idx += copy_length;
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
int SFMT19937Generator<CF>::fillBits32(int64_t n, uint32_t buffer[]) {
  // SFMT19937 already generates uniformly distributed integers in [0, 2^32]
  return fillBits(n, buffer);
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int SFMT19937Generator<CF>::fillBits64(int64_t n, uint64_t buffer[]) {
  return fillBits32(2 * n, reinterpret_cast<uint32_t *>(buffer));
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

static void
xorStatesInPlace(std::array<poly128_t, SFMT19937State::N> &result,
                 const std::array<poly128_t, SFMT19937State::N> &other) {
  for (size_t i = 0; i < SFMT19937State::N; i++) {
    result[i] = result[i] ^ other[i];
  }
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
void SFMT19937Generator<CF>::advanceOnceInPlace(
    std::array<poly128_t, SFMT19937State::N> &x) {

  auto x_2 = vld1q_u32((uint32_t *)&x[N - 2]);
  auto x_1 = vld1q_u32((uint32_t *)&x[N - 1]);

  const auto x_n = vld1q_u8((uint8_t *)&x[0]);
  const auto x_nm = vld1q_u32((uint32_t *)&x[M]);

  const auto x_a = veorq_u8(vextq_u8(vdupq_n_u8(0), x_n, 15), x_n);
  const auto x_b = vreinterpretq_u8_u32(vandq_u32(vshrq_n_u32(x_nm, 11), mask));
  const auto x_c = vextq_u8(vreinterpretq_u8_u32(x_2), vdupq_n_u8(0), 1);
  const auto x_d = vreinterpretq_u8_u32(vshlq_n_u32(x_1, 18));

  vst1q_u8((uint8_t *)&x[0], x_a ^ x_b ^ x_c ^ x_d);

  std::rotate(x.begin(), x.begin() + 1, x.end());
}

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
int SFMT19937Generator<CF>::skipAhead(uint64_t n, const uint64_t nskip[]) {
  if (n > 1) {
    return VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED;
  }

  const auto newIdx = 1 + ((state.current_idx + nskip[0] - 1) % 4);
  const auto nStateUpdates = (state.current_idx + nskip[0] - 1) / 4;

  DynamicArray<char> g = sfmt19937SkipAheadPolynomial(nStateUpdates);

  //
  // Taking the skip ahead polynomial g, get the sum (xor) of each i'th state.x
  // where the i'th coefficient of g is non-zero.
  //
  auto stateCopy = state.x;
  decltype(state.x) resultState = {};
  for (size_t i = 0; i < g.size(); i++) {
    if (g[i]) {
      xorStatesInPlace(resultState, stateCopy);
    }
    advanceOnceInPlace(stateCopy);
  }

  state.x = resultState;
  state.current_idx = newIdx;

  return VSL_ERROR_OK;
}

} // namespace openrng
