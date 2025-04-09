/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/neon/ars5_neon.hpp>

namespace openrng {

inline uint128_t do_bijection(const QuadCounterBasedState<uint128_t> &state);

template <CPUFeatures CF>
  requires(NeonOrAes<CF>)
int ARS5Generator<CF>::initialise(const int64_t n,
                                  const unsigned int params[]) {
  for (int i = 0; i < 4; ++i)
    state.k.data.u32[i] = n > i ? params[i] : 0;
  for (int i = 0; i < 4; ++i)
    state.c.data.u32[i] = n > i + 4 ? params[i + 4] : 0;
  state.off = 0;
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrAes<CF>)
int ARS5Generator<CF>::fillBits(int64_t n, uint32_t dst[]) {
  fill_array<uint32_t, do_bijection>(n, dst);
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrAes<CF>)
int ARS5Generator<CF>::fill(int64_t n, float dst[]) {
  fill_array<float, do_bijection>(n, dst);
  return VSL_ERROR_OK;
}

template <CPUFeatures CF>
  requires(NeonOrAes<CF>)
int ARS5Generator<CF>::fill(int64_t n, double dst[]) {
  fill_array<double, do_bijection>(n, dst);
  return VSL_ERROR_OK;
}

} // namespace openrng
