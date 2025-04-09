/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/sfmt19937.hpp>
#include <utils/dynamic_array.hpp>
#include <utils/ring_buffer.hpp>

#include <arm_neon.h>

namespace openrng {

struct SFMT19937StateNeon {
  constexpr static int64_t N = 156;
  uint32_t current_idx = N * 4;
  std::array<poly128_t, N> x = {};
};

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
class SFMT19937Generator<CF> : public CopyableGenerator<SFMT19937StateNeon> {

  constexpr static uint32x4_t mask{0xdfffffefU, 0xddfecb7fU, 0xbffaffffU,
                                   0xbffffff6U};
  constexpr static int64_t N = SFMT19937StateNeon::N;
  constexpr static int64_t M = 122;

  template <typename T>
  int advance(int64_t n, T buffer[], const T l, const T u);
  static void advanceOnceInPlace(ring_buffer<poly128_t, N> &x);
  template <typename TransformFunc> void refresh_state(TransformFunc transform);

public:
  SFMT19937Generator(int64_t brng) : CopyableGenerator{brng} {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t, const unsigned int[]);

  int fillBits(int64_t n, uint32_t[]) final;
  int fill(int64_t n, float[]) final;
  int fill(int64_t n, double[]) final;
  int fill(int64_t n, float buffer[], const float l, const float u) final;
  int fill(int64_t n, double buffer[], const double l, const double u) final;
  int skipAhead(uint64_t n, const uint64_t[]) final;
  int setStride(uint64_t stride) final;
};

} // namespace openrng
