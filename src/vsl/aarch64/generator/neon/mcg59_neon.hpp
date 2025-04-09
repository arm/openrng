/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/mcg59.hpp>

namespace openrng {

template <>
class Mcg59Generator<CPUFeatures::neon> : public CopyableGenerator<Mcg59State> {
public:
  Mcg59Generator(int64_t brng) : CopyableGenerator{brng} {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t n, const unsigned int params[]);

  int fillBits(int64_t n, uint32_t[]) final;
  int fillBits32(int64_t n, uint32_t[]) final;
  int fill(int64_t n, float[]) final;
  int fill(int64_t n, double[]) final;
  int skipAhead(uint64_t n, const uint64_t nskip[]) final;
  int setStride(uint64_t stride) final;

private:
  constexpr static int unroll_count = 24;
  constexpr static uint64_t m = 1LL << 59;  // (2 ** 59)
  constexpr static uint64_t m_mask = m - 1; // ((2 ** 59) - 1)

  void advance();

  template <std::floating_point T>
  void fillArrayUnrolled(T *dst)
    requires(unroll_count == 24);
};

} // namespace openrng
