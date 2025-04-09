/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <utils/mt_skip_ahead_data.hpp>

#include <cstring>

namespace openrng {

namespace {

template <typename T, int64_t N>
static void copyStates(std::array<T, N> &result, ring_buffer<T, N> &state) {
  const T *buffer = state.data();
  const size_t pos = state.get_position();
  for (size_t i = pos; i < N; i++) {
    result[i - pos] = buffer[i];
  }
  for (size_t i = 0; i < pos; i++) {
    result[N - pos + i] = buffer[i];
  }
}

template <typename T, int64_t N>
static void xorStatesInPlace(std::array<T, N> &result,
                             const std::array<T, N> &other) {
  for (size_t i = 0; i < N; i++) {
    result[i] ^= other[i];
  }
}

template <typename T, int64_t N>
static void xorStatesInPlace(ring_buffer<T, N> &result,
                             const std::array<T, N> &other) {
  T *buffer = result.data();
  const size_t pos = result.get_position();
  for (size_t i = pos; i < N; i++) {
    buffer[i] ^= other[i - pos];
  }
  for (size_t i = 0; i < pos; i++) {
    buffer[i] ^= other[N - pos + i];
  }
}

static inline bool get_coefficient(const DynamicArray<uint64_t> &g,
                                   const uint64_t i) {
  return 1 & g[i / 64] >> (i % 64);
}

/**
 * Extract a window of size q from g: g[i - 1], ..., g[i - q].
 */
static inline uint64_t get_window(const DynamicArray<uint64_t> &g,
                                  const int64_t i, const int64_t q) {
  uint64_t window = 0;
  for (int j = 1; j <= q; ++j) {
    window = (window << 1) ^ get_coefficient(g, i - j);
  }
  return window;
}

/**
 * Compute jump indices d_j and polynomial decompositions h_j(z) from a list of
 * coefficients g.
 *
 * Returns m, the number of windows that can fit within the coefficients.
 *
 * Reference:
 *   H. Haramoto and M. Matsumoto,
 *   "Efficient Jump Ahead for F2-Linear Random Number Generators"
 *   http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/ARTICLES/jumpf2-printed.pdf
 */
template <int64_t q, int64_t max_m>
int64_t slidingWindowSetup(const DynamicArray<uint64_t> &g,
                           std::array<uint64_t, max_m + 1> &h,
                           std::array<uint64_t, max_m> &d) {
  int64_t m = 0;
  int64_t i = g.size() * 64 - 1;

  // i >= q
  for (; i >= q; --i) {
    if (get_coefficient(g, i)) {
      m++;
      d[m] = i - q;
      h[m] = get_window(g, i, q);
      i -= q;
    }
  }

  // i < q
  h[m + 1] = get_window(g, i + 1, i + 1);

  return m;
}

/**
 * Returns the ith Gray code.
 */
static inline uint64_t graycode(const uint64_t i) { return i ^ (i >> 1); }

/**
 * Compute 2^q vectors h(A)x, where h(z) are the decomposed polynomials each of
 * degree q as defined in the paper.
 *
 * This returns the resulting states from advancing the state x by a window of
 * coefficients of size q. The vectors are enumerated by using Gray codes to be
 * efficient, as this step needs to be recomputed for each initial state x.
 *
 * Reference:
 *   H. Haramoto and M. Matsumoto,
 *   "Efficient Jump Ahead for F2-Linear Random Number Generators"
 *   http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/ARTICLES/jumpf2-printed.pdf
 */
template <typename T, int64_t N, int64_t q>
std::array<std::array<T, N>, 1 << q>
computeWindowSkips(const std::array<T, N> &x,
                   void (*advanceOnceInPlace)(ring_buffer<T, N> &)) {
  std::array<std::array<T, N>, 1 << q> y;
  std::array<std::array<T, N>, q> Aqx; // Aqx[i] = A^i * x

  ring_buffer<T, N> temp = {x};
  for (int i = 0; i < q; ++i) {
    copyStates<T, N>(Aqx[i], temp);
    advanceOnceInPlace(temp);
  }
  copyStates<T, N>(y[0], temp);

  for (int i = 1; i < (1 << q); ++i) {
    auto diff = graycode(i) ^ graycode(i - 1);
    int k = 0;
    while (diff >>= 1)
      k++;
    y[graycode(i)] = y[graycode(i - 1)];
    xorStatesInPlace<T, N>(y[graycode(i)], Aqx[k]);
  }
  return y;
}

} // namespace

/**
 * Skip ahead for MT19937 and SFMT19937 using the sliding window algorithm.
 *
 * Updates the state `x` after skipping ahead for `nSkips`. Takes in the cached
 * coefficients and function pointer for `advanceOnceInPlace`.
 */
template <typename T, int64_t N>
void mt_skip_ahead(std::array<T, N> &x,
                   void (*advanceOnceInPlace)(ring_buffer<T, N> &),
                   const uint64_t nSkips,
                   const std::array<DynamicArray<uint64_t>, 64> &coeffs) {
  uint64_t skip = nSkips;

  for (int k = 0; skip; ++k, skip >>= 1) {
    if ((skip & 1) == 0) {
      continue;
    }

    ring_buffer<T, N> resultState = {};

    // The code below follows the sliding window algorithm as described in:
    //  H. Haramoto and M. Matsumoto,
    //  "Efficient Jump Ahead for F2-Linear Random Number Generators"
    //  http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/ARTICLES/jumpf2-printed.pdf

    // Window Size
    constexpr int64_t q = 5;
    // max_m is the upper bound of the number of windows m,
    // where m <= ceil(19937 / (q + 1))
    constexpr int64_t max_m = 1 + (19936 / (q + 1));
    // h holds the polynomial decompositions of each window
    // d holds the index offsets at the end of each window
    std::array<uint64_t, max_m + 1> h = {};
    std::array<uint64_t, max_m> d = {};

    const auto m = slidingWindowSetup<q, max_m>(coeffs[k], h, d);
    const auto y = computeWindowSkips<T, N, q>(x, advanceOnceInPlace);

    xorStatesInPlace<T, N>(resultState, y[h[1]]);
    for (int j = 2; j <= m; ++j) {
      for (uint64_t i = 0; i < d[j - 1] - d[j]; ++i) {
        advanceOnceInPlace(resultState);
      }
      xorStatesInPlace<T, N>(resultState, y[h[j]]);
    }

    // y[h[1]] is already added if m = 0
    if (m > 0) {
      for (uint64_t i = 0; i < d[m]; ++i) {
        advanceOnceInPlace(resultState);
      }
      xorStatesInPlace<T, N>(resultState, y[h[m + 1]]);
    }
    // Cancel out the first term for the last window where size < q
    xorStatesInPlace<T, N>(resultState, y[0]);

    copyStates<T, N>(x, resultState);
  }
}

} // namespace openrng
