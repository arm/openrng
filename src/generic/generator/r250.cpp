/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/r250.hpp>

#include <cstring>

namespace openrng {

int R250Generator::initialise(const int64_t n, const unsigned int params[]) {
  const auto data = state.x.data();

  static_assert(sizeof(params[0]) == sizeof(data[0]));
  if (n >= 250) {
    memcpy(&data[0], &params[0], 250 * sizeof(params[0]));
    return VSL_ERROR_OK;
  }

  unsigned int seed;
  if (n == 0 || params[0] == 0) {
    seed = 1;
  } else {
    seed = params[0];
  }

  data[0] = 69069 * seed;
  for (uint32_t i = 1; i < N; ++i) {
    data[i] = 69069 * data[i - 1];
  }

  const uint32_t upper_mask = 0Xffffffff;
  const uint32_t diagonal_mask = 0X80000000;

  for (uint32_t k = 0; k < 32; k++) {
    const auto idx = 7 * k + 3;
    // Set under-diagonal bits to 0
    data[idx] &= upper_mask >> k;
    // Set diagonal bits to 1
    data[idx] |= diagonal_mask >> k;
  }
  return VSL_ERROR_OK;
}

uint32_t R250Generator::advance() {
  const uint32_t nextElem = state.x[q] ^ state.x[p];
  state.x.push_back(nextElem);
  return nextElem;
}

int R250Generator::fillBits(int64_t n, uint32_t buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = advance();
  }
  return VSL_ERROR_OK;
}

int R250Generator::fillBits32(int64_t, uint32_t[]) {
  return VSL_RNG_ERROR_BRNG_NOT_SUPPORTED;
}

int R250Generator::fillBits64(int64_t, uint64_t[]) {
  return VSL_RNG_ERROR_BRNG_NOT_SUPPORTED;
}

int R250Generator::fill(int64_t n, float buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = (float)advance() / (1ULL << 32);
  }
  return VSL_ERROR_OK;
}

int R250Generator::fill(int64_t n, double buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = (double)advance() / (1ULL << 32);
  }
  return VSL_ERROR_OK;
}

int R250Generator::setStride(uint64_t) {
  return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
}

int R250Generator::skipAhead(uint64_t, const uint64_t[]) {
  return VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED;
}
} // namespace openrng