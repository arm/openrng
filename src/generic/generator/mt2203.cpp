/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/mt2203.hpp>
#include <generator/mt2203_params.hpp>
#include <generator/mt_initialization.hpp>
#include <utils/mt_skip_ahead_data.hpp>

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
int MT2203Generator<CPUFeatures::generic>::initialise(
    const int64_t n, const unsigned int params[]) {
  state.a = MT2203_PARAMS[brngId - VSL_BRNG_MT2203].a;
  state.b = MT2203_PARAMS[brngId - VSL_BRNG_MT2203].b;
  state.c = MT2203_PARAMS[brngId - VSL_BRNG_MT2203].c;
  return mt_initialise_by_array<N>(n, params, state.x.data());
}

uint32_t MT2203Generator<CPUFeatures::generic>::advance() {
  // Step 1: compute next element of x
  auto x = (state.x[N] & 0xffffffe0) | (state.x[N - 1] & 0x1f);
  x = state.x[N - M] ^ ((x & 0x1) ? (x >> 1) ^ state.a : x >> 1);

  // Update x buffer
  state.x.push_back(x);

  // Step 2: compute next element of y
  auto y = x;
  y ^= y >> 12;
  y ^= (y << 7) & state.b;
  y ^= (y << 15) & state.c;
  y ^= y >> 18;

  return y;
}

int MT2203Generator<CPUFeatures::generic>::fillBits(int64_t n,
                                                    uint32_t buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = advance();
  }
  return VSL_ERROR_OK;
}

int MT2203Generator<CPUFeatures::generic>::fill(int64_t n, float buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = (float)advance() / (float)(1ULL << 32);
  }
  return VSL_ERROR_OK;
}

int MT2203Generator<CPUFeatures::generic>::fill(int64_t n, double buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = (double)advance() / (double)(1ULL << 32);
  }
  return VSL_ERROR_OK;
}

int MT2203Generator<CPUFeatures::generic>::setStride(uint64_t) {
  return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
}

int MT2203Generator<CPUFeatures::generic>::skipAhead(uint64_t,
                                                     const uint64_t[]) {
  return VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED;
}
} // namespace openrng
