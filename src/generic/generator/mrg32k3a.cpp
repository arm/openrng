/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/mrg32k3a.hpp>
#include <utils/helpers.hpp>

namespace openrng {

int MRG32k3aGenerator<CPUFeatures::generic>::initialise(
    const int64_t n, const unsigned int params[]) {
  state.x[0] = n >= 1 ? params[0] % m_1 : 1;
  state.x[1] = n >= 2 ? params[1] % m_1 : 1;
  state.x[2] = n >= 3 ? params[2] % m_1 : 1;

  state.y[0] = n >= 4 ? params[3] % m_2 : 1;
  state.y[1] = n >= 5 ? params[4] % m_2 : 1;
  state.y[2] = n >= 6 ? params[5] % m_2 : 1;

  if (state.x[0] == 0 && state.x[1] == 0 && state.x[2] == 0) {
    state.x[0] = 1;
  }

  if (state.y[0] == 0 && state.y[1] == 0 && state.y[2] == 0) {
    state.y[0] = 1;
  }
  return VSL_ERROR_OK;
}

uint32_t MRG32k3aGenerator<CPUFeatures::generic>::advance() {
  // Advance state vectors once & compute an output element
  state.x = matrix_x * state.x;
  state.y = matrix_y * state.y;
  return (state.x[2] - state.y[2].data).data;
}

int MRG32k3aGenerator<CPUFeatures::generic>::fillBits(int64_t n,
                                                      uint32_t buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = advance();
  }
  return VSL_ERROR_OK;
}

int MRG32k3aGenerator<CPUFeatures::generic>::fillBits32(int64_t, uint32_t[]) {
  return VSL_RNG_ERROR_BRNG_NOT_SUPPORTED;
}

int MRG32k3aGenerator<CPUFeatures::generic>::fillBits64(int64_t, uint64_t[]) {
  return VSL_RNG_ERROR_BRNG_NOT_SUPPORTED;
}

int MRG32k3aGenerator<CPUFeatures::generic>::fill(int64_t n, float buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = (float)advance() / (float)m_1;
  }
  return VSL_ERROR_OK;
}

int MRG32k3aGenerator<CPUFeatures::generic>::fill(int64_t n, double buffer[]) {
  for (int i = 0; i < n; i++) {
    buffer[i] = (double)advance() / (double)m_1;
  }
  return VSL_ERROR_OK;
}

/**
 * Skip ahead the MRG32K3A state by the number of steps specified in nskip.
 *
 * This implementation works by modelling the state update as a linear
 * transformation, where we advance the state by constructing a transition
 * matrix and applying it to a state vector (for both the X & Y state).
 *
 * Advancing the state by N steps is therefore equivalent to raising the
 * transition matrix to a power of N & applying it to the state vector.
 * This can be done in O(log N) steps by computing the matrix power via
 * exponentiation by squaring.
 *
 * Advancing by the number specified by the nskip array as:
 *   nskip[0] * 2^(64*0) + nskip[1] * 2^(64*1) + ... + nskip[i] * 2^(64*i)
 * is done efficiently by raising our transformation matrices to a power
 * of 2^(64*i) by squaring it 64*i times.
 *
 * For more information on how this implementation operates, please see:
 *  L'ecuyer, Pierre, et al.
 *  "An Object-Oriented Random-Number Package with Many Long Streams and
 *  Substreams." Operations research 50.6 (2002): 1073-1075.
 *  https://doi.org/10.1287/opre.50.6.1073.358
 */
int MRG32k3aGenerator<CPUFeatures::generic>::skipAhead(uint64_t n,
                                                       const uint64_t nskip[]) {
  for (uint64_t i = 0; i < n; i++) {
    if (nskip[i] == 0)
      continue;

    auto skip_matrix_x{matrix_x};
    auto skip_matrix_y{matrix_y};

    // Skip ahead 2^(64*i) steps by squaring our matrix 64*i times
    for (uint64_t j = 0; j < 64 * i; j++) {
      skip_matrix_x = skip_matrix_x * skip_matrix_x;
      skip_matrix_y = skip_matrix_y * skip_matrix_y;
    }

    // Raise matrix to a power of nskip[i] such that we advance
    // nskip[i] * 2^(64*i) steps
    skip_matrix_x = skip_matrix_x.pow(nskip[i]);
    skip_matrix_y = skip_matrix_y.pow(nskip[i]);

    // Advance state by applying transformation matrices
    state.x = skip_matrix_x * state.x;
    state.y = skip_matrix_y * state.y;
  }

  return VSL_ERROR_OK;
}

int MRG32k3aGenerator<CPUFeatures::generic>::setStride(uint64_t) {
  return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
}

} // namespace openrng
