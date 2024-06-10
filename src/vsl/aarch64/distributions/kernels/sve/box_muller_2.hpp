/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/box_muller_2.hpp>

namespace openrng::distribution::gaussian {

template <> struct BoxMuller2<CPUFeatures::sve> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *, const int64_t, T[], const T, const T);
};

} // namespace openrng::distribution::gaussian
