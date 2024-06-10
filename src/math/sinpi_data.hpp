/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <cstdint>

namespace openrng::math {
namespace {

struct sinpif_data {
  constexpr static float poly[6] = {0x1.921fb6p1f, -0x1.4abbcep2f,
                                    0x1.466bc6p1f, -0x1.32d2ccp-1f,
                                    0x1.50783p-4f, -0x1.e30750p-8f};
};

constexpr static float sinpif_poly(float x) {
  constexpr auto P = sinpif_data::poly;
  /* Pairwise Horner evaluation.  */
  const float p01 = P[0] + x * P[1];
  const float p23 = P[2] + x * P[3];
  const float p45 = P[4] + x * P[5];
  const float x2 = x * x;
  float y = p23 + x2 * p45;
  y = p01 + y * x2;
  return y;
}

struct sinpi_data {
  constexpr static double poly[10] = {
      0x1.921fb54442d184p1,  -0x1.4abbce625be53p2,   0x1.466bc6775ab16p1,
      -0x1.32d2cce62dc33p-1, 0x1.507834891188ep-4,   -0x1.e30750a28c88ep-8,
      0x1.e8f48308acda4p-12, -0x1.6fc0032b3c29fp-16, 0x1.af86ae521260bp-21,
      -0x1.012a9870eeb7dp-25};
};

constexpr static double sinpi_poly(double x) {
  constexpr auto P = sinpi_data::poly;
  /* Pairwise Horner evaluation.  */
  const double p01 = P[0] + x * P[1];
  const double p23 = P[2] + x * P[3];
  const double p45 = P[4] + x * P[5];
  const double p67 = P[6] + x * P[7];
  const double p89 = P[8] + x * P[9];

  const double x2 = x * x;
  double y = p67 + x2 * p89;
  y = p45 + y * x2;
  y = p23 + y * x2;
  y = p01 + y * x2;
  return y;
}

} // namespace
} // namespace openrng::math
