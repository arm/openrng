/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/neon/quad_counter_based_generator_neon.hpp>
#include <generator/philox4x3210.hpp>

namespace openrng {

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
class Philox4x3210Generator<CF>
    : public QuadCounterBasedGenerator<CF, std::array<uint32_t, 2>> {
public:
  Philox4x3210Generator(int64_t brng)
      : QuadCounterBasedGenerator<CF, std::array<uint32_t, 2>>{brng} {}

  using QuadCounterBasedGenerator<CF, std::array<uint32_t, 2>>::initialise;
  int initialise(const int64_t n, const unsigned int params[]);

  int fillBits(int64_t n, uint32_t dst[]);

  int fill(int64_t n, float dst[]);

  int fill(int64_t n, double dst[]);
};

} // namespace openrng
