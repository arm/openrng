/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/copyable_generator.hpp>
#include <utils/cpu_features.hpp>

namespace openrng {

struct Mcg31m1State {
  uint64_t a = 1132489760;
  uint32_t x = 0;
};

template <CPUFeatures> class Mcg31m1Generator {};

template <>
class Mcg31m1Generator<CPUFeatures::generic>
    : public CopyableGenerator<Mcg31m1State> {
  constexpr static inline uint64_t m = 0x7FFFFFFF; // (2 ** 31 - 1)

  void advance();

public:
  Mcg31m1Generator(int64_t brng) : CopyableGenerator{brng} {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t n, const unsigned int params[]);

  int fillBits(int64_t n, uint32_t[]) final;
  int fillBits32(int64_t n, uint32_t[]) final;
  int fillBits64(int64_t n, uint64_t[]) final;
  int fill(int64_t n, float[]) final;
  int fill(int64_t n, double[]) final;
  int skipAhead(uint64_t n, const uint64_t nskip[]) final;
  int setStride(uint64_t stride) final;
};

} // namespace openrng
