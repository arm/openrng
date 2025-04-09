/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/copyable_generator.hpp>
#include <utils/cpu_features.hpp>
#include <utils/ring_buffer.hpp>

namespace openrng {

struct MT2203State {
  constexpr static int64_t N = 69;
  ring_buffer<uint32_t, N> x = {};
  uint32_t a, b, c;
};

template <CPUFeatures> class MT2203Generator {};

template <>
class MT2203Generator<CPUFeatures::generic>
    : public CopyableGenerator<MT2203State> {

  constexpr static int64_t N = MT2203State::N;
  constexpr static int64_t M = 34;

  uint32_t advance();

public:
  MT2203Generator(int64_t brng) : CopyableGenerator{brng} {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t, const unsigned int[]);

  int fillBits(int64_t n, uint32_t[]) final;
  int fill(int64_t n, float[]) final;
  int fill(int64_t n, double[]) final;
  int setStride(uint64_t) final;
  int skipAhead(uint64_t, const uint64_t[]) final;
};

} // namespace openrng
