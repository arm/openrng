/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/mcg31m1.hpp>

namespace openrng {

template <>
class Mcg31m1Generator<CPUFeatures::neon>
    : public CopyableGenerator<Mcg31m1State> {
public:
  Mcg31m1Generator() : CopyableGenerator{VSL_BRNG_MCG31} {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t n, const unsigned int params[]);

  int fillBits(int64_t n, uint32_t[]) final;
  int fillBits32(int64_t n, uint32_t[]) final;
  int fillBits64(int64_t n, uint64_t[]) final;
  int fill(int64_t n, float[]) final;
  int fill(int64_t n, double[]) final;
  int fill(int64_t n, float buffer[], const float l, const float u) final;
  int fill(int64_t n, double buffer[], const double l, const double u) final;
  int skipAhead(uint64_t n, const uint64_t nskip[]) final;
  int setStride(uint64_t stride) final;

private:
  static constexpr int unroll_count = 20;
  static constexpr uint64_t m = 0x7FFFFFFF; // (2 ** 31 - 1)

  void advance();

  template <std::floating_point T>
  void fillArrayUnrolled(T *dst, const T scale, const T l)
    requires(unroll_count == 20);
};

} // namespace openrng
