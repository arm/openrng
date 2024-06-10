/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/copyable_generator.hpp>
#include <generator/philox4x3210.hpp>
#include <utils/uint128.hpp>

namespace openrng {

template <CPUFeatures CF>
  requires(NeonOrSha3<CF>)
class Philox4x3210Generator<CF> : public CopyableGenerator<Philox4x3210State> {
public:
  Philox4x3210Generator() : CopyableGenerator{VSL_BRNG_PHILOX4X32X10} {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t, const unsigned int[]);

  int fillBits(int64_t n, uint32_t[]) final;
  int fillBits32(int64_t n, uint32_t[]) final;
  int fillBits64(int64_t n, uint64_t[]) final;
  int fill(int64_t n, float[]) final;
  int fill(int64_t n, double[]) final;
  int skipAhead(uint64_t n, const uint64_t nskip[]) final;
  int setStride(uint64_t stride) final;

private:
  template <typename T> void fill_array(int64_t, T *);
  void advance(uint64_t);
  uint128_t get_philox() const;
};

} // namespace openrng
