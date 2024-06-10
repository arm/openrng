/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/mt19937.hpp>
#include <utils/mt_skip_ahead.hpp>

namespace openrng {

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
int MT19937Generator<CPUFeatures::generic>::initialise(
    const int64_t n, const unsigned int params[]) {
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

uint32_t MT19937Generator<CPUFeatures::generic>::advance() {
  // Step 1: compute next element of x
  auto x = (state.x[N] & 0x80000000) | (state.x[N - 1] & 0x7FFFFFFF);
  x = state.x[N - M] ^ ((x & 0x1) ? (x >> 1) ^ a : x >> 1);

  // Update x buffer
  state.x.push_back(x);

  // Step 2: compute next element of y
  auto y = x;
  y ^= y >> 11;
  y ^= (y << 7) & 0x9D2C5680;
  y ^= (y << 15) & 0xEFC60000;
  y ^= y >> 18;

  return y;
}

int MT19937Generator<CPUFeatures::generic>::fillBits(int64_t n,
                                                     uint32_t buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = advance();
  }
  return VSL_ERROR_OK;
}

int MT19937Generator<CPUFeatures::generic>::fillBits32(int64_t n,
                                                       uint32_t buffer[]) {
  // MT19937 already generates uniformly distributed integers in [0, 2^32]
  return fillBits(n, buffer);
}

int MT19937Generator<CPUFeatures::generic>::fillBits64(int64_t n,
                                                       uint64_t buffer[]) {
  return fillBits32(2 * n, reinterpret_cast<uint32_t *>(buffer));
}

int MT19937Generator<CPUFeatures::generic>::fill(int64_t n, float buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = (float)advance() / (float)(1ULL << 32);
  }
  return VSL_ERROR_OK;
}

int MT19937Generator<CPUFeatures::generic>::fill(int64_t n, double buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = (double)advance() / (double)(1ULL << 32);
  }
  return VSL_ERROR_OK;
}

int MT19937Generator<CPUFeatures::generic>::setStride(uint64_t) {
  return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
}

static void
xorStatesInPlace(ring_buffer<uint32_t, MT19937State::N> &result,
                 const ring_buffer<uint32_t, MT19937State::N> &other) {
  for (size_t i = 0; i < MT19937State::N; i++) {
    result[i] ^= other[i];
  }
}

int MT19937Generator<CPUFeatures::generic>::skipAhead(uint64_t n,
                                                      const uint64_t nskip[]) {
  if (n > 1) {
    return VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED;
  }

  DynamicArray<char> g = mt19937SkipAheadPolynomial(nskip[0]);

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
  }

  state.x = resultState;

  return VSL_ERROR_OK;
}

} // namespace openrng
