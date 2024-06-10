/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/copyable_generator.hpp>
#include <utils/cpu_features.hpp>
#include <utils/ring_buffer.hpp>
#include <utils/uint128.hpp>

namespace openrng {

struct SFMT19937State {
  constexpr static int64_t N = 156;
  uint32_t current_idx = 4;
  ring_buffer<uint128_t, N> x = {};
};

template <CPUFeatures> class SFMT19937Generator {};

template <>
class SFMT19937Generator<CPUFeatures::generic>
    : public CopyableGenerator<SFMT19937State> {

  constexpr static std::array<uint32_t, 4> mask{0xdfffffefU, 0xddfecb7fU,
                                                0xbffaffffU, 0xbffffff6U};
  constexpr static int64_t N = SFMT19937State::N;
  constexpr static int64_t M = 122;

  uint32_t advance();

public:
  SFMT19937Generator() : CopyableGenerator{VSL_BRNG_SFMT19937} {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t, const unsigned int[]);

  int fillBits(int64_t n, uint32_t[]) final;
  int fillBits32(int64_t n, uint32_t[]) final;
  int fillBits64(int64_t n, uint64_t[]) final;
  int fill(int64_t n, float[]) final;
  int fill(int64_t n, double[]) final;
  int skipAhead(uint64_t n, const uint64_t[]) final;
  int setStride(uint64_t stride) final;
};

} // namespace openrng
