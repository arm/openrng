/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <utils/names_map.hpp>

#if defined(__aarch64__) && defined(__linux__)

#include <asm/hwcap.h>
#include <sys/auxv.h>

// Kernel headers may be too old for HWCAP2_RNG
#ifndef HWCAP2_RNG
#define HWCAP2_RNG (1 << 16)
#endif

static inline bool trng_is_supported() {
  const static bool is_supported = (getauxval(AT_HWCAP2) & HWCAP2_RNG) != 0;
  return is_supported;
}

#elif defined(__x86_64__)

#include <cpuid.h>

#define RDRAND_BIT (1 << 30)

static inline bool trng_is_supported() {
  unsigned int eax, ebx, ecx, edx;
  __get_cpuid(1, &eax, &ebx, &ecx, &edx);
  const static bool is_supported = (ecx & RDRAND_BIT) != 0;
  return is_supported;
}

#else

static inline bool trng_is_supported() { return false; }

#endif

namespace {

bool is_deterministic(int brngId) { return brngId != VSL_BRNG_NONDETERM; }

#define ALLOW_NONDETERM_UNSUPPORTED_IF_NO_TRNG(brng_id, err)                   \
  if (err == VSL_RNG_ERROR_NONDETERM_NOT_SUPPORTED) {                          \
    REQUIRE(brng_id == VSL_BRNG_NONDETERM);                                    \
    REQUIRE(!trng_is_supported());                                             \
    SKIP("TRNG not supported");                                                \
  }

} // namespace
