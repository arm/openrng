/*
 * SPDX-FileCopyrightText: <text>Copyright 2024, 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/geometric.hpp>
#include <math/aarch64/log.hpp>

namespace openrng::distribution::geometric {

template <> struct ICDF<CPUFeatures::sve> {
  static int fill(Generator *, const int64_t, int32_t[], const double);
};

} // namespace openrng::distribution::geometric
