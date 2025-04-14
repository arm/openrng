/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/neon/ars5_neon_impl.hpp>
#include <utils/ars.hpp>

namespace openrng {
template class ARS5Generator<CPUFeatures::neon>;

inline uint128_t do_bijection(const QuadCounterBasedState<uint128_t> &state) {
  return compute_ars5(state.c, state.k);
}
} // namespace openrng
