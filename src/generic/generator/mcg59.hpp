/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/copyable_generator.hpp>
#include <utils/cpu_features.hpp>

#include <cmath>

namespace openrng {

struct Mcg59State {
  uint64_t a = pow(13, 13);
  uint64_t x = 0;
};

template <CPUFeatures> class Mcg59Generator {};

template <>
class Mcg59Generator<CPUFeatures::generic>
    : public CopyableGenerator<Mcg59State> {
  constexpr static uint64_t m = 1LL << 59; // (2 ** 59)

  void advance();

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
};

} // namespace openrng
