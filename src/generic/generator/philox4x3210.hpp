/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/copyable_generator.hpp>
#include <utils/cpu_features.hpp>
#include <utils/uint128.hpp>

namespace openrng {

struct Philox4x3210State {
  uint128_t c;
  /* k0 and k1 are keys - they cannot be marked const because they are set at
   * initialization, but should not be changed afterwards.  */
  uint32_t k0{}, k1{};
  /* Offset used when filling array begins in the middle of a value of c.  */
  uint32_t off{};
};

template <CPUFeatures> class Philox4x3210Generator {};

template <>
class Philox4x3210Generator<CPUFeatures::generic>
    : public CopyableGenerator<Philox4x3210State> {
private:
  void advance(uint64_t);
  uint128_t get_philox() const;
  template <typename T> void fill_array(int64_t, T *);

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
};

} // namespace openrng
