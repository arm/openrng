/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/nondeterministic.hpp>

namespace openrng {

template <>
class NondeterministicGenerator<CPUFeatures::trng>
    : public NondeterministicGenerator<CPUFeatures::generic> {
public:
  NondeterministicGenerator()
      : NondeterministicGenerator<CPUFeatures::generic>() {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t, const unsigned int[]);

  int fillBits(int64_t, uint32_t[]) final;
  int fillBits32(int64_t, uint32_t[]) final;
  int fillBits64(int64_t, uint64_t[]) final;
  int fill(int64_t, float[]) final;
  int fill(int64_t, double[]) final;
};

} // namespace openrng
