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

struct EmptyState {};

template <CPUFeatures> class NondeterministicGenerator {};

template <>
class NondeterministicGenerator<CPUFeatures::generic>
    : public CopyableGenerator<EmptyState> {
public:
  NondeterministicGenerator(int64_t brng) : CopyableGenerator{brng} {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t, const unsigned int[]);

  int skipAhead(uint64_t, const uint64_t[]) final;
  int setStride(uint64_t) final;
};

} // namespace openrng
