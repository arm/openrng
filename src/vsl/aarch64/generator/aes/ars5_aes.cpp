/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/neon/ars5_neon_impl.hpp>

#include <arm_neon.h>

namespace openrng {
template class ARS5Generator<CPUFeatures::aes>;

namespace {
inline uint8x16_t next_round_key(const uint8x16_t k) {
  uint64x2_t temp = vreinterpretq_u64_u8(k);
  const uint64_t key_const[2] = {0x9E3779B97F4A7C15llu, 0xBB67AE8584CAA73Bllu};
  temp = vaddq_u64(temp, vld1q_u64(key_const));
  return vreinterpretq_u8_u64(temp);
}

inline uint128_t compute_ars5_aes(const uint128_t c, const uint128_t k) {
  uint8x16_t ci = vld1q_u8(c.data.u8);
  uint8x16_t ki = vld1q_u8(k.data.u8);
  for (int64_t i = 1; i < 5; ++i) {
    // Compute AES round
    // The key is added (xor) in the AESE instruction internally, before
    // shifting rows and substituting bytes.
    ci = vaesmcq_u8(vaeseq_u8(ci, ki));
    // Get next round key, which will be added in the next AESE instruction
    ki = next_round_key(ki);
  }
  // Last round without mix column
  ci = vaeseq_u8(ci, ki);
  // Add the key for the last time
  ki = next_round_key(ki);
  ci = veorq_u8(ci, ki);
  uint128_t result;
  vst1q_u8(result.data.u8, ci);
  return result;
}
} // namespace

inline uint128_t do_bijection(const QuadCounterBasedState<uint128_t> &state) {
  return compute_ars5_aes(state.c, state.k);
}
} // namespace openrng
