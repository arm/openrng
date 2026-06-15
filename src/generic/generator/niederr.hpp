/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/niederr_default_initialization.hpp>
#include <generator/qrng_base_generator.hpp>

namespace openrng {

class NiederrGenerator : public QRNGBaseGenerator {
  int initialise_direction_nums(const uint32_t *polynomials);

  constexpr static int max_default_dimensions = 318;

public:
  NiederrGenerator(int64_t brng) : QRNGBaseGenerator{brng} {}

  constexpr static uint32_t calcc2_maxv =
      directions_per_dimension + niederr_default_initialization::maxdeg;

  using QRNGBaseGenerator::initialise;
  int initialise(const int64_t n, const unsigned int params[]);
};

} // namespace openrng
