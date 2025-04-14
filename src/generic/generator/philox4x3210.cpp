/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <cstring>
#include <generator/philox4x3210.hpp>
#include <utils/philox.hpp>

namespace openrng {

namespace {
inline uint128_t
do_bijection(const QuadCounterBasedState<std::array<uint32_t, 2>> &state) {
  return philox_10(state.k[0], state.k[1], state.c);
}
} // namespace

int Philox4x3210Generator<CPUFeatures::generic>::initialise(
    const int64_t n, const unsigned int params[]) {
  for (int i = 0; i < 4; i++)
    state.c.data.u32[i] = n > i + 2 ? params[i + 2] : 0;

  state.k[0] = n == 0 ? 0 : params[0];
  state.k[1] = n > 1 ? params[1] : 0;

  state.off = 0;

  return VSL_ERROR_OK;
}

int Philox4x3210Generator<CPUFeatures::generic>::fillBits(int64_t n,
                                                          uint32_t dst[]) {
  fill_array<uint32_t, do_bijection>(n, dst);
  return VSL_ERROR_OK;
}

int Philox4x3210Generator<CPUFeatures::generic>::fill(int64_t n, float dst[]) {
  fill_array<float, do_bijection>(n, dst);
  return VSL_ERROR_OK;
}

int Philox4x3210Generator<CPUFeatures::generic>::fill(int64_t n, double dst[]) {
  fill_array<double, do_bijection>(n, dst);
  return VSL_ERROR_OK;
}

} // namespace openrng
