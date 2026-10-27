/*
 * SPDX-FileCopyrightText: <text>Copyright 2024, 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/rayleigh.hpp>
#include <math/aarch64/log.hpp>

namespace openrng::distribution::rayleigh {

template <> struct ICDF<CPUFeatures::sve> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *, const int64_t, T[], const T, const T);
};

} // namespace openrng::distribution::rayleigh