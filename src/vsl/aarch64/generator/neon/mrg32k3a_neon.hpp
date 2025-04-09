/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <array>
#include <generator/copyable_generator.hpp>
#include <generator/mrg32k3a.hpp>
#include <utils/galois_field_matrix.hpp>

namespace openrng {

struct MRG32k3aStateNeon {
  /**
   * The number of elements calculated sequentially in the generators hot loop.
   */
  constexpr static uint64_t nLanes = 2;

  /**
   * The number of previous elements we need to preserve between iterations.
   *
   * In the following example, f and e are calculated from d, c, b, and a. To
   * calculate h and g, we need to preserve d and c.
   *
   *    [ d, c, b, a ] -> [ f, e, d, c ]
   */
  constexpr static uint64_t nExtraElements = 2;

  constexpr static uint64_t m_1 = (1LL << 32) - 209;
  constexpr static uint64_t m_2 = (1LL << 32) - 22853;

  // X state vector:
  // [ x_(n-1), x_(n-2), x_(n-3) ]
  std::array<uint32_t, nLanes + nExtraElements> x = {};

  // Y state vector:
  // [ y_(n-1), y_(n-2), y_(n-3) ]
  std::array<uint32_t, nLanes + nExtraElements> y = {};
};

template <>
class MRG32k3aGenerator<CPUFeatures::neon>
    : public CopyableGenerator<MRG32k3aStateNeon> {

  constexpr static uint64_t m_1 = MRG32k3aStateNeon::m_1;
  constexpr static uint64_t m_2 = MRG32k3aStateNeon::m_2;

  // [ a_11, a_12, a_13 ]
  constexpr static inline std::array<GaloisFieldElement<m_1>, 3> coeffs_x{
      m_1 - 810728, 1403580, 0};
  // [ a_21, a_22, a_23 ]
  constexpr static inline std::array<GaloisFieldElement<m_2>, 3> coeffs_y{
      m_2 - 1370589, 0, 527612};

  // Transition matrix for the X state.
  // Applying this to the X state vector advances it by one step.
  // [ a_11 | a_12 | a_13 ]
  // [ 1    | 0    | 0    ]
  // [ 0    | 1    | 0    ]
  constexpr static GaloisFieldMatrix<3, m_1> matrix_x{
      {0, 1, 0, 0, 0, 1, coeffs_x[0], coeffs_x[1], coeffs_x[2]}
  };

  // Transition matrix for the Y state.
  // Applying this to the Y state vector advances it by one step.
  // [ a_21 | a_22 | a_23 ]
  // [ 1    | 0    | 0    ]
  // [ 0    | 1    | 0    ]
  constexpr static GaloisFieldMatrix<3, m_2> matrix_y{
      {0, 1, 0, 0, 0, 1, coeffs_y[0], coeffs_y[1], coeffs_y[2]}
  };

  uint32_t advance(int64_t n, uint32_t buffer[]);
  void backfill(const std::array<GaloisFieldElement<m_1>, 3> &x,
                const std::array<GaloisFieldElement<m_2>, 3> &y);

public:
  MRG32k3aGenerator(int64_t brng) : CopyableGenerator{brng} {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t n, const unsigned int params[]);

  int fillBits(int64_t n, uint32_t[]) final;
  int fillBits32(int64_t n, uint32_t[]) final;
  int fillBits64(int64_t n, uint64_t[]) final;
  int fill(int64_t n, float[]) final;
  int fill(int64_t n, double[]) final;
  int fill(int64_t n, float buffer[], const float l, const float u) final;
  int fill(int64_t n, double buffer[], const double l, const double u) final;
  int skipAhead(uint64_t n, const uint64_t nskip[]) final;
  int setStride(uint64_t stride) final;
};

} // namespace openrng
