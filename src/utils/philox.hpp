/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <utils/uint128.hpp>

namespace openrng {

static inline void bump_keys(uint32_t &k0, uint32_t &k1) {
  k0 += 0x9E3779B9;
  k1 += 0xBB67AE85;
}

static inline uint32_t mulhi(uint64_t a, uint64_t b) { return (a * b) >> 32; }
static inline uint32_t mullo(uint32_t a, uint32_t b) { return a * b; }

static inline void philox_oneround(uint128_t &c, uint32_t k0, uint32_t k1) {
  const auto R0 = c.data.u32[0], L0 = c.data.u32[1], R1 = c.data.u32[2],
             L1 = c.data.u32[3];
  auto &[R0_, L0_, R1_, L1_] = c.data.u32;
  L1_ = mullo(R0, 0xD2511F53);
  R1_ = mulhi(R0, 0xD2511F53) ^ k1 ^ L1;
  L0_ = mullo(R1, 0xCD9E8D57);
  R0_ = mulhi(R1, 0xCD9E8D57) ^ k0 ^ L0;
}

static inline uint128_t philox_10(uint32_t k0, uint32_t k1, uint128_t c) {
  for (int i = 0; i < 9; i++) {
    philox_oneround(c, k0, k1);
    bump_keys(k0, k1);
  }
  philox_oneround(c, k0, k1);
  return c;
}
} // namespace openrng
