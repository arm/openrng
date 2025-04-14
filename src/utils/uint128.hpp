/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <array>

namespace openrng {

class uint128_t {
public:
  union data {
    uint8_t u8[16];
    uint32_t u32[4];
    uint64_t u64[2];
    __uint128_t u128;
  } data;

  constexpr uint128_t(){};
  constexpr uint128_t(const std::array<uint8_t, 16> data_in) {
    for (int i = 0; i < 16; ++i)
      this->data.u8[i] = data_in[i];
  };
  constexpr uint128_t(const std::array<uint32_t, 4> data_in)
      : data{
            .u32 = {data_in[0], data_in[1], data_in[2], data_in[3]}
  } {};
  constexpr uint128_t(const std::array<uint64_t, 2> data_in)
      : data{
            .u64 = {data_in[0], data_in[1]}
  } {};
  constexpr uint128_t(const __uint128_t data_in) : data{.u128 = data_in} {};

  uint128_t operator&(uint128_t rhs) const { return data.u128 & rhs.data.u128; }
  uint128_t operator^(uint128_t rhs) const { return data.u128 ^ rhs.data.u128; }
  uint128_t operator+(uint128_t rhs) const { return data.u128 + rhs.data.u128; }
  uint128_t operator>>(int n) const { return data.u128 >> n; }
  uint128_t operator<<(int n) const { return data.u128 << n; }
  uint128_t &operator+=(const uint128_t &other) {
    return *this = *this + other;
  }

  /**
   * Interpret the 128-bit integer as a group of 32-bit uints
   * and shift each element left by `n`.
   */
  uint128_t quad_asl(int n) const {
    uint128_t ret;
    for (int i = 0; i < 4; i++) {
      ret.data.u32[i] = data.u32[i] << n;
    }
    return ret;
  }

  /**
   * Interpret the 128-bit integer as a group of 32-bit uints
   * and shift each element right by `n`.
   */
  uint128_t quad_asr(int n) const {
    uint128_t ret;
    for (int i = 0; i < 4; i++) {
      ret.data.u32[i] = data.u32[i] >> n;
    }
    return ret;
  }
};

} // namespace openrng
