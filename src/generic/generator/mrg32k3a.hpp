/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <array>
#include <generator/copyable_generator.hpp>
#include <utils/cpu_features.hpp>
#include <utils/galois_field_matrix.hpp>

namespace openrng {

struct MRG32k3aState {
  /**
   * The number of elements calculated sequentially in the generators hot loop.
   */
  constexpr static uint64_t nLanes = 1;

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
  std::array<GaloisFieldElement<m_1>, nLanes + nExtraElements> x = {};

  // Y state vector:
  // [ y_(n-1), y_(n-2), y_(n-3) ]
  std::array<GaloisFieldElement<m_2>, nLanes + nExtraElements> y = {};
};

template <CPUFeatures> class MRG32k3aGenerator {};

template <>
class MRG32k3aGenerator<CPUFeatures::generic>
    : public CopyableGenerator<MRG32k3aState> {

  constexpr static uint64_t m_1 = MRG32k3aState::m_1;
  constexpr static uint64_t m_2 = MRG32k3aState::m_2;

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

  uint32_t advance();

public:
  MRG32k3aGenerator() : CopyableGenerator{VSL_BRNG_MRG32K3A} {}

  using CopyableGenerator::initialise;
  int initialise(const int64_t n, const unsigned int params[]);

  int fillBits(int64_t n, uint32_t[]) final;
  int fillBits32(int64_t n, uint32_t[]) final;
  int fillBits64(int64_t n, uint64_t[]) final;
  int fill(int64_t n, float[]) final;
  int fill(int64_t n, double[]) final;
  int skipAhead(uint64_t n, const uint64_t nskip[]) final;
  int setStride(uint64_t stride) final;
};

} // namespace openrng
