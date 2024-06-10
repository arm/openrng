/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/nondeterministic.hpp>

namespace openrng {

int NondeterministicGenerator<CPUFeatures::generic>::initialise(
    const int64_t, const unsigned int[]) {
  return VSL_RNG_ERROR_NONDETERM_NOT_SUPPORTED;
}

int NondeterministicGenerator<CPUFeatures::generic>::skipAhead(
    uint64_t, const uint64_t[]) {
  return VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED;
}

int NondeterministicGenerator<CPUFeatures::generic>::setStride(uint64_t) {
  return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
}

} // namespace openrng
