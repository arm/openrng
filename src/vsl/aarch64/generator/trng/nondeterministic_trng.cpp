/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <arm_acle.h>
#include <cstring>

#include <generator/trng/nondeterministic_trng.hpp>

namespace openrng {

int NondeterministicGenerator<CPUFeatures::trng>::initialise(
    const int64_t, const unsigned int[]) {
  return VSL_ERROR_OK;
}

int NondeterministicGenerator<CPUFeatures::trng>::fillBits(int64_t n,
                                                           uint32_t dst[]) {
  const int err = fillBits64(n / 2, (uint64_t *)dst);
  if (n % 2) {
    uint64_t tail;
    fillBits64(1, &tail);
    dst[n - 1] = tail & 0xff'ff'ff'ff;
  }
  return err;
}

int NondeterministicGenerator<CPUFeatures::trng>::fillBits64(int64_t n,
                                                             uint64_t dst[]) {
  for (int i = 0; i < n; i++)
    if (__rndr(dst + i) != 0)
      return VSL_RNG_ERROR_BAD_STREAM;
  return VSL_ERROR_OK;
}

int NondeterministicGenerator<CPUFeatures::trng>::fill(int64_t n, float dst[]) {
  const int err = fillBits32(n, (uint32_t *)dst);
  for (int i = 0; i < n; i++) {
    uint32_t ix;
    memcpy(&ix, dst + i, sizeof(float));
    dst[i] = (float)(int)ix / 0x1p32f + 0.5f;
  }
  return err;
}

int NondeterministicGenerator<CPUFeatures::trng>::fill(int64_t n,
                                                       double dst[]) {
  const auto err = fillBits64(n, (uint64_t *)dst);
  for (int i = 0; i < n; i++) {
    uint64_t ix;
    memcpy(&ix, dst + i, sizeof(double));
    dst[i] = (double)(int)ix / 0x1p32 + 0.5;
  }
  return err;
}

} // namespace openrng
