/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <utils/cpu_features.hpp>

#ifdef __linux__
#include <asm/hwcap.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <sys/auxv.h>
#endif

namespace openrng {

#ifdef __linux__

static inline CPUFeatures getMaxAllowedISA() {
  const char *maxISAEnvVar = getenv("OPENRNG_MAX_ISA");
  if (maxISAEnvVar != NULL) {
    if (strcmp(maxISAEnvVar, "generic") == 0)
      return CPUFeatures::generic;
    if (strcmp(maxISAEnvVar, "trng") == 0)
      return CPUFeatures::trng;
    if (strcmp(maxISAEnvVar, "neon") == 0)
      return CPUFeatures::neon;
    if (strcmp(maxISAEnvVar, "sha3") == 0)
      return CPUFeatures::sha3;
  }
  assert(maxISAEnvVar == NULL || strcmp(maxISAEnvVar, "sve") == 0);
  /* Either the environment variable was not set or it was set to SVE. Either
     way, return SVE - note that this can only restrict the features on the
     host, so SVE will still only be used if present.  */
  return CPUFeatures::sve;
}

bool is_enabled(unsigned long which_hwcap, long feat, CPUFeatures feat_enum) {
  return (getauxval(which_hwcap) & feat) != 0 &&
         feat_enum <= getMaxAllowedISA();
}

// Kernel headers may be too old for HWCAP2_RNG
#ifndef HWCAP2_RNG
#define HWCAP2_RNG (1 << 16)
#endif

const bool SupportedFeatures::hasTrng =
    is_enabled(AT_HWCAP2, HWCAP2_RNG, CPUFeatures::trng);
const bool SupportedFeatures::hasNeon =
    is_enabled(AT_HWCAP, HWCAP_ASIMD, CPUFeatures::neon);
const bool SupportedFeatures::hasSha3 =
    is_enabled(AT_HWCAP, HWCAP_SHA3, CPUFeatures::sha3);
const bool SupportedFeatures::hasSve =
    is_enabled(AT_HWCAP, HWCAP_SVE, CPUFeatures::sve);

#else // non-Linux

/* Note: we currently don't support any feature detection on non-Linux
 * platforms. This means any optimised generators that rely on optional
 * CPU features will not be exposed on macOS & Windows.
 * TODO: revisit this & add in feature detection for macOS
 */
const bool SupportedFeatures::hasTrng = false;
const bool SupportedFeatures::hasNeon = true;
const bool SupportedFeatures::hasSha3 = false;
const bool SupportedFeatures::hasSve = false;

#endif

} // namespace openrng
