/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/mt2203.hpp>

namespace openrng {

struct MT2203StateNeon {
  constexpr static int64_t N = 69;
  std::array<uint32_t, N> x, tempered = {};
  uint32_t a, b, c, index = 0;
};

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
class MT2203Generator<CF> : public CopyableGenerator<MT2203StateNeon> {

  constexpr static int64_t N = MT2203State::N;
  constexpr static int64_t M = 34;

  template <typename T> uint32_t advance(int64_t n, T buffer[], T, T);
  template <typename T> void refresh_state(T[], T, T);

public:
  MT2203Generator(int64_t brng) : CopyableGenerator{brng} {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t, const unsigned int[]);

  int fillBits(int64_t n, uint32_t[]) final;
  int fill(int64_t n, float[]) final;
  int fill(int64_t n, double[]) final;
  int setStride(uint64_t) final;
  int skipAhead(uint64_t, const uint64_t[]) final;
  int fill(int64_t n, float buffer[], const float l, const float u) final;
  int fill(int64_t n, double buffer[], const double l, const double u) final;
};

} // namespace openrng
