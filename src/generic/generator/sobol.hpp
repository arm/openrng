/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/qrng_base_generator.hpp>

namespace openrng {

class SobolGenerator : public QRNGBaseGenerator {
  int initialise_direction_nums(const uint32_t *, const uint32_t *,
                                const uint32_t, bool);

  constexpr static int max_default_dimensions = 40;

public:
  SobolGenerator(int64_t brng) : QRNGBaseGenerator{brng} {}

  using QRNGBaseGenerator::initialise;
  int initialise(const int64_t n, const unsigned int params[]);
};

} // namespace openrng
