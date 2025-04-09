/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <algorithm>
#include <cstdint>

#include <openrng.h>

namespace openrng {

/*
 * Initialize state array according to the 'initialization by array' procedure
 * described in:
 *  M. Matsumoto and T. Nishimura, "Mersenne Twister with improved
 *  initialization"
 *  http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/MT2002/emt19937ar.html
 */
template <int64_t N>
int mt_initialise_by_array(int64_t n, const unsigned int params[],
                           uint32_t *data) {
  data[0] = 19650218;
  for (int i = 1; i < N; i++) {
    data[i] = 1812433253ULL * (data[i - 1] ^ (data[i - 1] >> 30)) + i;
  }

  int i = 1;
  for (int j = 0, k = 0; k < std::max(n, N); k++) {
    data[i] ^= 1664525ULL * (data[i - 1] ^ (data[i - 1] >> 30));
    data[i] += params[j] + j;
    i = i == N - 1 ? 1 : i + 1;
    j = j == n - 1 ? 0 : j + 1;
    if (i == 1) {
      data[0] = data[N - 1];
    }
  }

  for (int k = 0; k < N - 1; k++) {
    data[i] ^= 1566083941ULL * (data[i - 1] ^ (data[i - 1] >> 30));
    data[i] -= i;
    i = i == N - 1 ? 1 : i + 1;
    if (i == 1) {
      data[0] = data[N - 1];
    }
  }

  data[0] = 1 << 31;
  return VSL_ERROR_OK;
}
} // namespace openrng
