/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

namespace openrng {

/**
 * Supported CPU Features.
 *
 * When adding to this list, keep in mind that CPUFeatures is ordered.
 */
enum class CPUFeatures { generic, trng };

inline bool operator<=(CPUFeatures a, CPUFeatures b) {
  return static_cast<int>(a) <= static_cast<int>(b);
}

struct SupportedFeatures {
  const static bool hasTrng;
};

} // namespace openrng
