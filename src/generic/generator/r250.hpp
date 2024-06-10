/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/copyable_generator.hpp>
#include <utils/ring_buffer.hpp>

namespace openrng {

struct R250State {
  constexpr static uint32_t N = 250;
  ring_buffer<uint32_t, N> x = {};
};

class R250Generator : public CopyableGenerator<R250State> {

  constexpr static uint32_t N = R250State::N;
  constexpr static uint32_t p = 250;

  // There's a typo in the GSL documentation that specifies q = 103, yet their
  // implementation uses q = 147 which returns the sequence in "time-reversed
  // order". (Note: 103 == 250 - 147)
  constexpr static uint32_t q = 147;

  uint32_t advance();

public:
  R250Generator() : CopyableGenerator{VSL_BRNG_R250} {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t n, const unsigned int params[]);

  int fillBits(int64_t n, uint32_t[]) final;
  int fillBits32(int64_t n, uint32_t[]) final;
  int fillBits64(int64_t n, uint64_t[]) final;
  int fill(int64_t n, float[]) final;
  int fill(int64_t n, double[]) final;
  int setStride(uint64_t stride) final;
  int skipAhead(uint64_t n, const uint64_t nskip[]) final;
};

} // namespace openrng