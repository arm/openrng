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

namespace openrng {

struct MT19937State {
  constexpr static int64_t N = 624;
  ring_buffer<uint32_t, N> x = {};
};

template <CPUFeatures> class MT19937Generator {};

template <>
class MT19937Generator<CPUFeatures::generic>
    : public CopyableGenerator<MT19937State> {

  constexpr static int64_t N = MT19937State::N;
  constexpr static int64_t M = 397;
  constexpr static uint32_t a = 0x9908B0DF;

  uint32_t advance();

public:
  MT19937Generator() : CopyableGenerator{VSL_BRNG_MT19937} {}

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
