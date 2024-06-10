/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <math/neon/util.hpp>

#include <arm_neon.h>

namespace openrng::math::neon {
namespace {

template <typename T> struct erfinv_data;

template <> struct erfinv_data<float> {
  constexpr static float32x4_t Plo = {-0x1.a31268p+3f, -0x1.fc0252p-4f,
                                      0x1.ac9048p+4f, 0x1.119d44p+0f};
  constexpr static float32x4_t PQ = {-0x1.293ff6p+3f, -0x1.f59ee2p+0f,
                                     -0x1.8265eep+3f, -0x1.69952p-4f};
  constexpr static float32x4_t Qhi = {0x1.ef5eaep+4f, 0x1.c7b7d2p-1f,
                                      -0x1.12665p+4f, -0x1.167d7p+1f};
  constexpr static float32x4_t P29_3 = dup(0x1.b13626p-2f);
  constexpr static float32x4_t Q_50[2] = {dup(0x1.3d7dacp-3f),
                                          dup(0x1.629e5p+0f)};
  constexpr static float32x4_t P_50[6] = {
      dup(0x1.3d8948p-3f),  dup(0x1.61f9eap+0f), dup(0x1.61c6bcp-1f),
      dup(-0x1.20c9f2p+0f), dup(0x1.5c704cp-1f), dup(-0x1.50c6bep-3f)};
  constexpr static uint8x16_t idxlo = {0, 1, 2, 3, 0, 1, 2, 3,
                                       0, 1, 2, 3, 0, 1, 2, 3};
  constexpr static uint8x16_t idxhi = {8, 9, 10, 11, 8, 9, 10, 11,
                                       8, 9, 10, 11, 8, 9, 10, 11};
  constexpr static float32x4_t tailshift = dup(-0.87890625f);
};

template <> struct erfinv_data<double> {
  constexpr static double P[8][2] = {
      {0x1.007ce8f01b2e8p+4,  -0x1.f3596123109edp-7},
      {-0x1.6b23cc5c6c6d7p+6, 0x1.60b8fe375999ep-2 },
      {0x1.74e5f6ceb3548p+7,  -0x1.779bb9bef7c0fp+1},
      {-0x1.5200bb15cc6bbp+7, 0x1.786ea384470a2p+3 },
      {0x1.05d193233a849p+6,  -0x1.6a7c1453c85d3p+4},
      {-0x1.148c5474ee5e1p+3, 0x1.31f0fc5613142p+4 },
      {0x1.689181bbafd0cp-3,  -0x1.5ea6c007d4dbbp+2},
      {0,                     0x1.e66f265ce9e5p-3  }
  };

  constexpr static double Q[7][2] = {
      {0x1.d8fb0f913bd7bp+3,  -0x1.636b2dcf4edbep-7},
      {-0x1.6d7f25a3f1c24p+6, 0x1.0b5411e2acf29p-2 },
      {0x1.a450d8e7f4cbbp+7,  -0x1.3413109467a0bp+1},
      {-0x1.bc3480485857p+7,  0x1.563e8136c554ap+3 },
      {0x1.ae6b0c504ee02p+6,  -0x1.7b77aab1dcafbp+4},
      {-0x1.499dfec1a7f5fp+4, 0x1.8a3e174e05ddcp+4 },
      {0x1p+0,                -0x1.4075c56404eecp+3}
  };
  constexpr static float64x2_t tailshift = dup(-0.87890625);
  constexpr static uint8x16_t idx = {0, 1, 2, 3, 4, 5, 6, 7,
                                     0, 1, 2, 3, 4, 5, 6, 7};
  constexpr static float64x2_t P_57[9] = {
      dup(0x1.b874f9516f7f1p-14), dup(0x1.5921f2916c1c4p-7),
      dup(0x1.145ae7d5b8fa4p-2),  dup(0x1.29d6dcc3b2fb7p+1),
      dup(0x1.cabe2209a7985p+2),  dup(0x1.11859f0745c4p+3),
      dup(0x1.b7ec7bc6a2ce5p+2),  dup(0x1.d0419e0bb42aep+1),
      dup(0x1.c5aa03eef7258p-1)};
  constexpr static float64x2_t Q_57[10] = {
      dup(0x1.b8747e12691f1p-14), dup(0x1.59240d8ed1e0ap-7),
      dup(0x1.14aef2b181e2p-2),   dup(0x1.2cd181bcea52p+1),
      dup(0x1.e6e63e0b7aa4cp+2),  dup(0x1.65cf8da94aa3ap+3),
      dup(0x1.7e5c787b10a36p+3),  dup(0x1.0626d68b6cea3p+3),
      dup(0x1.065c5f193abf6p+2),  dup(0x1p+0)};
};

} // namespace
} // namespace openrng::math::neon
