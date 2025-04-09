/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <openrng.h>
#include <string>

static std::string brngToString(openrng_int_t method) {
  if (method >= VSL_BRNG_MT2203 && method < VSL_BRNG_MT2203 + 6024) {
    return "MT2203 + " + std::to_string(method - VSL_BRNG_MT2203);
  }

  switch (method) {
  case VSL_BRNG_MCG31:
    return "MCG31";
  case VSL_BRNG_MRG32K3A:
    return "MRG32K3A";
  case VSL_BRNG_R250:
    return "R250";
  case VSL_BRNG_MCG59:
    return "MCG59";
  case VSL_BRNG_WH:
    return "WH";
  case VSL_BRNG_SOBOL:
    return "SOBOL";
  case VSL_BRNG_NIEDERR:
    return "NIEDERR";
  case VSL_BRNG_MT19937:
    return "MT19937";
  case VSL_BRNG_IABSTRACT:
    return "IABSTRACT";
  case VSL_BRNG_DABSTRACT:
    return "DABSTRACT";
  case VSL_BRNG_SABSTRACT:
    return "SABSTRACT";
  case VSL_BRNG_SFMT19937:
    return "SFMT19937";
  case VSL_BRNG_NONDETERM:
    return "NONDETERM";
  case VSL_BRNG_ARS5:
    return "ARS5";
  case VSL_BRNG_PHILOX4X32X10:
    return "PHILOX4X32X10";
  default:
    return "Name not found";
  }
}
