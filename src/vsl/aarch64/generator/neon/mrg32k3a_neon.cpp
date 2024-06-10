/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <algorithm>

#include <generator/neon/mrg32k3a_neon.hpp>
#include <utils/dynamic_array.hpp>
#include <utils/helpers.hpp>

namespace openrng {

int MRG32k3aGenerator<CPUFeatures::neon>::initialise(
    const int64_t n, const unsigned int params[]) {

  std::array<GaloisFieldElement<m_1>, 3> x = {};
  std::array<GaloisFieldElement<m_2>, 3> y = {};

  x[0] = n >= 1 ? params[0] % m_1 : 1;
  x[1] = n >= 2 ? params[1] % m_1 : 1;
  x[2] = n >= 3 ? params[2] % m_1 : 1;

  y[0] = n >= 4 ? params[3] % m_2 : 1;
  y[1] = n >= 5 ? params[4] % m_2 : 1;
  y[2] = n >= 6 ? params[5] % m_2 : 1;

  if (x[0] == 0 && x[1] == 0 && x[2] == 0) {
    x[0] = 1;
  }

  if (y[0] == 0 && y[1] == 0 && y[2] == 0) {
    y[0] = 1;
  }

  backfill(x, y);

  return VSL_ERROR_OK;
}

/**
 * Restore previous state from the 3 most recent values of both x and y.
 *
 * Runs the generator in reverse to fill in some old state. Running the
 * generator forward, and precalculating future state would also be a valid
 * tactic, but it would add a bit of extra logic to advancing the state, so
 * initialising backwards ends up being neater.
 */
void MRG32k3aGenerator<CPUFeatures::neon>::backfill(
    const std::array<GaloisFieldElement<m_1>, 3> &x,
    const std::array<GaloisFieldElement<m_2>, 3> &y) {

  const auto [a0, a1, a2] = coeffs_x;
  const auto [b0, b1, b2] = coeffs_y;

  const auto toData = [](auto e) { return e.data; };
  std::transform(x.rbegin(), x.rend(), state.x.rbegin(), toData);
  std::transform(y.rbegin(), y.rend(), state.y.rbegin(), toData);

  for (int id = state.x.size() - 4; id >= 0; id--) {
    // Recursively solves the following for x0 & y0
    //
    //   a0 * x0 + a1 * x1 + a2 * x2 = x3 mod M
    //
    const GaloisFieldElement<m_1> x1 = state.x[id + 1];
    const GaloisFieldElement<m_1> x2 = state.x[id + 2];
    const GaloisFieldElement<m_1> x3 = state.x[id + 3];
    const GaloisFieldElement<m_2> y1 = state.y[id + 1];
    const GaloisFieldElement<m_2> y2 = state.y[id + 2];
    const GaloisFieldElement<m_2> y3 = state.y[id + 3];
    state.x[id] = ((x3 - x2 * a2 - x1 * a1) / a0).data;
    state.y[id] = ((y3 - y2 * b2 - y1 * b1) / b0).data;
  }
}

/**
 * Equivalent to x % m_1.
 *
 * This is an optimisation the compiler usually does, but clang-17 has a bug
 * that prevents it using fast modulus when unrolling.
 *
 * It approximates (1 / m_1) as ceil(c / 2**95), ie. fixed point with scaling
 * factor of 2**95. Don't mistake this as a generic solution for doing a fast
 * modulus. The scaling factor has been predetermined by the compiler and
 * usually a fixed point product either over or underestimates the exact value,
 * depending on rounding, but the compiler has also predetemined that no
 * adjustment is needed here.
 */
static inline uint64_t fastMod1(uint64_t x) {
  constexpr uint32_t m = MRG32k3aStateNeon::m_1;
  constexpr __uint128_t c = 1 + (__uint128_t(1) << 95) / m;
  return x - ((x * c) >> 95) * m;
}

/**
 * Equivalent to x % m_2.
 *
 * See fastMod1.
 */
static inline uint64_t fastMod2(uint64_t x) {
  constexpr uint32_t m = MRG32k3aStateNeon::m_2;
  constexpr __uint128_t c = 1 + (__uint128_t(1) << 94) / m;
  return x - ((x * c) >> 94) * m;
}

/**
 * Finds x mod m_1 in the range [0, 2 * m_1).
 *
 * An intermediate step of Barret Reduction. It approximates (1 / m_1) as
 * floor(c / 2**64), so division can be done with a single umulh. It's faster
 * than fastMod1 because it avoids the need for a right shift, but the answer
 * may be shifted by m_1. It's useful if you have intermediate values that you
 * know won't overflow.
 */
static inline uint64_t approxMod1(uint64_t x) {
  constexpr uint32_t m = MRG32k3aStateNeon::m_1;
  constexpr __uint128_t c = (__uint128_t(1) << 64) / m;
  return x - ((x * c) >> 64) * m;
}

/**
 * Finds x mod m_2 in the range [0, 2 * m_2).
 *
 * See approxMod1.
 */
static inline uint64_t approxMod2(uint64_t x) {
  constexpr uint32_t m = MRG32k3aStateNeon::m_2;
  constexpr __uint128_t c = (__uint128_t(1) << 64) / m;
  return x - ((x * c) >> 64) * m;
}

uint32_t MRG32k3aGenerator<CPUFeatures::neon>::advance(int64_t n,
                                                       uint32_t buffer[]) {

  int i = 0;
  uint64_t x0 = state.x[0];
  uint64_t x1 = state.x[1];
  uint64_t x2 = state.x[2];
  uint64_t x3 = state.x[3];
  uint64_t y0 = state.y[0];
  uint64_t y1 = state.y[1];
  uint64_t y2 = state.y[2];
  uint64_t y3 = state.y[3];

  for (; i < n - 1; i += 2) {
    constexpr static auto transform_x = matrix_x * matrix_x;
    constexpr static auto transform_y = matrix_y * matrix_y;

    constexpr uint64_t a0 = transform_x(2, 0).data;
    constexpr uint64_t a1 = transform_x(2, 1).data;
    constexpr uint64_t a2 = transform_x(2, 2).data;
    constexpr uint64_t b0 = transform_y(2, 0).data;
    constexpr uint64_t b1 = transform_y(2, 1).data;
    constexpr uint64_t b2 = transform_y(2, 2).data;

    auto x4 = fastMod1(approxMod1(a0 * x0) + approxMod1(a1 * x1) +
                       approxMod1(a2 * x2));
    auto x5 = fastMod1(approxMod1(a0 * x1) + approxMod1(a1 * x2) +
                       approxMod1(a2 * x3));
    auto y4 = fastMod2(approxMod2(b0 * y0) + approxMod2(b1 * y1) +
                       approxMod2(b2 * y2));
    auto y5 = fastMod2(approxMod2(b0 * y1) + approxMod2(b1 * y2) +
                       approxMod2(b2 * y3));

    x0 = x2;
    x1 = x3;
    x2 = x4;
    x3 = x5;
    y0 = y2;
    y1 = y3;
    y2 = y4;
    y3 = y5;

    buffer[i + 0] = fastMod1(m_1 + x4 - y4);
    buffer[i + 1] = fastMod1(m_1 + x5 - y5);
  }

  for (; i < n; i++) {
    // Advance state vectors once & compute an output element
    constexpr uint64_t a0 = coeffs_x[0].data;
    constexpr uint64_t a1 = coeffs_x[1].data;
    constexpr uint64_t a2 = coeffs_x[2].data;
    constexpr uint64_t b0 = coeffs_y[0].data;
    constexpr uint64_t b1 = coeffs_y[1].data;
    constexpr uint64_t b2 = coeffs_y[2].data;

    const auto x4 =
        fastMod1(fastMod1(a0 * x1) + fastMod1(a1 * x2) + fastMod1(a2 * x3));
    const auto y4 =
        fastMod2(fastMod2(b0 * y1) + fastMod2(b1 * y2) + fastMod2(b2 * y3));

    x0 = x1;
    x1 = x2;
    x2 = x3;
    x3 = x4;
    y0 = y1;
    y1 = y2;
    y2 = y3;
    y3 = y4;

    buffer[i] = fastMod1(m_1 + x4 - y4);
  }

  state.x[0] = x0;
  state.x[1] = x1;
  state.x[2] = x2;
  state.x[3] = x3;
  state.y[0] = y0;
  state.y[1] = y1;
  state.y[2] = y2;
  state.y[3] = y3;

  return VSL_ERROR_OK;
}

int MRG32k3aGenerator<CPUFeatures::neon>::fillBits(int64_t n,
                                                   uint32_t buffer[]) {
  return advance(n, buffer);
}

int MRG32k3aGenerator<CPUFeatures::neon>::fillBits32(int64_t, uint32_t[]) {
  return VSL_RNG_ERROR_BRNG_NOT_SUPPORTED;
}

int MRG32k3aGenerator<CPUFeatures::neon>::fillBits64(int64_t, uint64_t[]) {
  return VSL_RNG_ERROR_BRNG_NOT_SUPPORTED;
}

int MRG32k3aGenerator<CPUFeatures::neon>::fill(int64_t n, float buffer[]) {
  return fill(n, buffer, 0.0, 1.0);
}

int MRG32k3aGenerator<CPUFeatures::neon>::fill(int64_t n, double buffer[]) {
  return fill(n, buffer, 0.0, 1.0);
}

int MRG32k3aGenerator<CPUFeatures::neon>::fill(int64_t n, float buffer[],
                                               const float l, const float u) {
  static_assert(sizeof(float) == sizeof(uint32_t));
  auto buffer_u32 = reinterpret_cast<uint32_t *>(buffer);
  advance(n, buffer_u32);
  constexpr float norm = 1.0 / (float)(m_1);
  for (int i = 0; i < n; i++) {
    buffer[i] = ((float)buffer_u32[i] * norm) * (u - l) + l;
  }
  return VSL_ERROR_OK;
}

int MRG32k3aGenerator<CPUFeatures::neon>::fill(int64_t n, double buffer[],
                                               const double l, const double u) {
  DynamicArray<uint32_t> tmpBuffer(n);
  advance(n, tmpBuffer.data());

  constexpr double norm = 1.0 / (double)(m_1);

  for (int i = 0; i < n; i++) {
    buffer[i] = ((double)tmpBuffer[i] * norm) * (u - l) + l;
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
int MRG32k3aGenerator<CPUFeatures::neon>::skipAhead(uint64_t n,
                                                    const uint64_t nskip[]) {

  std::array<GaloisFieldElement<m_1>, 3> x;
  std::array<GaloisFieldElement<m_2>, 3> y;

  std::copy(state.x.rbegin(), state.x.rbegin() + 3, x.rbegin());
  std::copy(state.y.rbegin(), state.y.rbegin() + 3, y.rbegin());

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
    x = skip_matrix_x * x;
    y = skip_matrix_y * y;
  }

  backfill(x, y);

  return VSL_ERROR_OK;
}

int MRG32k3aGenerator<CPUFeatures::neon>::setStride(uint64_t) {
  return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
}

} // namespace openrng
