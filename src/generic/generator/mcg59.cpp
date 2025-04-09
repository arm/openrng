/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/mcg59.hpp>
#include <utils/helpers.hpp>

namespace openrng {

int Mcg59Generator<CPUFeatures::generic>::initialise(
    const int64_t n, const unsigned int params[]) {
  uint64_t seed;
  if (n == 0) {
    seed = 1;
  } else if (n == 1) {
    seed = params[0];
  } else {
    seed = ((uint64_t)params[1] << 32) | params[0];
  }

  state.x = seed % m;

  if (state.x == 0) {
    state.x = 1;
  }

  return VSL_ERROR_OK;
}

void Mcg59Generator<CPUFeatures::generic>::advance() {
  state.x = (state.a * state.x) % m;
}

int Mcg59Generator<CPUFeatures::generic>::fillBits(int64_t n,
                                                   uint32_t buffer[]) {
  for (int64_t i = 0; i < n; i++) {
    reinterpret_cast<uint64_t *>(buffer)[i] = state.x;
    advance();
  }

  return VSL_ERROR_OK;
}

int Mcg59Generator<CPUFeatures::generic>::fillBits32(int64_t n,
                                                     uint32_t buffer[]) {
  for (int64_t i = 0; i < n; i++) {
    buffer[i] = state.x >> 27;
    advance();
  }

  return VSL_ERROR_OK;
}

int Mcg59Generator<CPUFeatures::generic>::fill(int64_t n, float buffer[]) {
  for (int64_t i = 0; i < n; i++) {
    buffer[i] = (float)state.x / (float)m;
    advance();
  }

  return VSL_ERROR_OK;
}

int Mcg59Generator<CPUFeatures::generic>::fill(int64_t n, double buffer[]) {
  for (int64_t i = 0; i < n; i++) {
    buffer[i] = (double)state.x / (double)m;
    advance();
  }

  return VSL_ERROR_OK;
}

int Mcg59Generator<CPUFeatures::generic>::setStride(uint64_t stride) {
  state.a = powMod(state.a, stride, m);

  return VSL_ERROR_OK;
}

int Mcg59Generator<CPUFeatures::generic>::skipAhead(uint64_t n,
                                                    const uint64_t nskip[]) {
  if (n > 1) {
    return VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED;
  }

  auto exponent = nskip[0];
  auto skipA = powMod(state.a, exponent, m);
  state.x = (skipA * state.x) % m;

  return VSL_ERROR_OK;
}

} // namespace openrng
