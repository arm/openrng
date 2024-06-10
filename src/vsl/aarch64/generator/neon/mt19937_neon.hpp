/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/copyable_generator.hpp>
#include <generator/mt19937.hpp>
#include <utils/ring_buffer.hpp>

namespace openrng {

struct MT19937StateNeon {
  constexpr static int64_t N = 624;
  std::array<uint32_t, N> x = {};
  std::array<uint32_t, N> y = {};
  size_t index = N;
};

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
class MT19937Generator<CF> : public CopyableGenerator<MT19937StateNeon> {

  constexpr static int64_t N = MT19937StateNeon::N;
  constexpr static int64_t M = 397;
  constexpr static uint32_t a = 0x9908B0DF;

  void refresh_state();
  void advanceOnceInPlace(std::array<uint32_t, N> &x);
  template <typename T>
  uint32_t advance(int64_t n, T buffer[], const T l, const T u);

  template <openrng::FloatingPoint T>
  void transform(T buffer[], uint32_t source[], const size_t length, const T l,
                 const T u);

public:
  MT19937Generator() : CopyableGenerator{VSL_BRNG_MT19937} {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t, const unsigned int[]);

  int fillBits(int64_t n, uint32_t[]) final;
  int fillBits32(int64_t n, uint32_t[]) final;
  int fillBits64(int64_t n, uint64_t[]) final;
  int fill(int64_t n, float[]) final;
  int fill(int64_t n, double[]) final;
  int fill(int64_t n, float buffer[], const float l, const float u) final;
  int fill(int64_t n, double buffer[], const double l, const double u) final;
  int skipAhead(uint64_t n, const uint64_t[]) final;
  int setStride(uint64_t stride) final;
};

} // namespace openrng
