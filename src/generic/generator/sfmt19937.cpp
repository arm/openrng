/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/sfmt19937.hpp>
#include <utils/mt_skip_ahead.hpp>

#include <cstring>

namespace openrng {

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
int SFMT19937Generator<CPUFeatures::generic>::initialise(
    const int64_t key_length, const unsigned int init_key[]) {
  int i, j, count;
  uint32_t r;
  constexpr auto lag = 11;
  constexpr auto size = N * 4;
  constexpr auto mid = (size - lag) / 2;

  // Get a pointer to uint32_t data
  auto *const data = reinterpret_cast<uint32_t *>(&state.x.data()[0].data);
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

uint32_t SFMT19937Generator<CPUFeatures::generic>::advance() {
  // Compute 128-bits at a time
  if (state.current_idx == 4) {
    // Reset index
    state.current_idx = 0;

    // Compute next 128-bit element of sequence
    const auto x_a = (state.x[N] << 8) ^ state.x[N];
    const auto x_b = state.x[N - M].quad_asr(11) & mask;
    const auto x_c = state.x[2] >> 8;
    const auto x_d = state.x[1].quad_asl(18);

    const auto x = x_a ^ x_b ^ x_c ^ x_d;

    // Update x buffer
    state.x.push_back(x);
  }

  // Return current 32-bit segment
  return state.x[1].data.u32[state.current_idx++];
}

int SFMT19937Generator<CPUFeatures::generic>::fillBits(int64_t n,
                                                       uint32_t buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = advance();
  }
  return VSL_ERROR_OK;
}

int SFMT19937Generator<CPUFeatures::generic>::fillBits32(int64_t n,
                                                         uint32_t buffer[]) {
  // SFMT19937 already generates uniformly distributed integers in [0, 2^32]
  return fillBits(n, buffer);
}

int SFMT19937Generator<CPUFeatures::generic>::fillBits64(int64_t n,
                                                         uint64_t buffer[]) {
  return fillBits32(2 * n, reinterpret_cast<uint32_t *>(buffer));
}

int SFMT19937Generator<CPUFeatures::generic>::fill(int64_t n, float buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = 0.5 + (float)(int32_t)advance() / (float)(1ULL << 32);
  }
  return VSL_ERROR_OK;
}

int SFMT19937Generator<CPUFeatures::generic>::fill(int64_t n, double buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = 0.5 + (double)(int32_t)advance() / (double)(1ULL << 32);
  }
  return VSL_ERROR_OK;
}

int SFMT19937Generator<CPUFeatures::generic>::setStride(uint64_t) {
  return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
}

static void
xorStatesInPlace(ring_buffer<uint128_t, SFMT19937State::N> &result,
                 const ring_buffer<uint128_t, SFMT19937State::N> &other) {
  for (size_t i = 0; i < SFMT19937State::N; i++) {
    result[i] = result[i] ^ other[i];
  }
}

int SFMT19937Generator<CPUFeatures::generic>::skipAhead(
    uint64_t n, const uint64_t nskip[]) {
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
  decltype(state.x) resultState = {};
  for (size_t i = 0; i < g.size(); i++) {
    if (g[i]) {
      xorStatesInPlace(resultState, state.x);
    }
    advance();
    advance();
    advance();
    advance();
  }

  state.x = resultState;
  state.current_idx = newIdx;

  return VSL_ERROR_OK;
}

} // namespace openrng
