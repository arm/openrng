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
enum class CPUFeatures { generic, trng, neon, sha3, aes, sve };

inline bool operator<=(CPUFeatures a, CPUFeatures b) {
  return static_cast<int>(a) <= static_cast<int>(b);
}

struct SupportedFeatures {
  const static bool hasTrng;
  const static bool hasNeon;
  const static bool hasSha3;
  const static bool hasAes;
  const static bool hasSve;
};

template <CPUFeatures CF>
concept NeonOrSha3 = CF == CPUFeatures::neon || CF == CPUFeatures::sha3;

template <CPUFeatures CF>
concept NeonOrAes = CF == CPUFeatures::neon || CF == CPUFeatures::aes;

} // namespace openrng
