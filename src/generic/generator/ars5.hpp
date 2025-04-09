/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/quad_counter_based_generator.hpp>
#include <utils/cpu_features.hpp>
#include <utils/uint128.hpp>

namespace openrng {

template <CPUFeatures> class ARS5Generator {};

template <>
class ARS5Generator<CPUFeatures::generic>
    : public QuadCounterBasedGenerator<CPUFeatures::generic, uint128_t> {
public:
  ARS5Generator(int64_t brng) : QuadCounterBasedGenerator{brng} {}

  using QuadCounterBasedGenerator::initialise;
  int initialise(const int64_t n, const unsigned int params[]);

  int fillBits(int64_t n, uint32_t dst[]);

  int fill(int64_t n, float dst[]);

  int fill(int64_t n, double dst[]);
};

} // namespace openrng
