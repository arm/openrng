/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/neon/mcg59_neon.hpp>
#include <utils/helpers.hpp>

namespace openrng {

int Mcg59Generator<CPUFeatures::neon>::initialise(const int64_t n,
                                                  const unsigned int params[]) {
  uint64_t seed;
  if (n == 0) {
    seed = 1;
  } else if (n == 1) {
    seed = params[0];
  } else {
    seed = ((uint64_t)params[1] << 32) | params[0];
  }

  state.x = seed & m_mask;

  if (state.x == 0) {
    state.x = 1;
  }

  return VSL_ERROR_OK;
}

int Mcg59Generator<CPUFeatures::neon>::fillBits(int64_t n, uint32_t buffer[]) {
  int i = 0;
  uint64_t *buffer64 = reinterpret_cast<uint64_t *>(buffer);
  const uint64_t a2 = state.a * state.a & m_mask;
  const uint64_t a4 = a2 * a2 & m_mask;
  const uint64_t a8 = a4 * a4 & m_mask;
  const uint64_t a12 = a8 * a4 & m_mask;
  const uint64_t a16 = a8 * a8 & m_mask;
  const uint64_t a20 = a16 * a4 & m_mask;

  for (; i + unroll_count < n; i += unroll_count) {
    const uint64_t x2 = state.x * a2 & m_mask;
    buffer64[i + 0] = state.x;
    buffer64[i + 1] = state.x * state.a & m_mask;
    buffer64[i + 2] = x2;
    buffer64[i + 3] = x2 * state.a & m_mask;

    for (int j = 0; j < 4; j++) {
      buffer64[i + 4 + j] = buffer64[i + j] * a4 & m_mask;
      buffer64[i + 8 + j] = buffer64[i + j] * a8 & m_mask;
      buffer64[i + 12 + j] = buffer64[i + j] * a12 & m_mask;
      buffer64[i + 16 + j] = buffer64[i + j] * a16 & m_mask;
      buffer64[i + 20 + j] = buffer64[i + j] * a20 & m_mask;
    }

    state.x = buffer64[i + unroll_count - 1] * state.a & m_mask;
  }

  const uint64_t a3 = state.a * a2 & m_mask;

  for (; i + 4 < n; i += 4) {
    buffer64[i + 0] = state.x;
    buffer64[i + 1] = state.a * state.x & m_mask;
    buffer64[i + 2] = a2 * state.x & m_mask;
    buffer64[i + 3] = a3 * state.x & m_mask;
    state.x = a4 * state.x & m_mask;
  }

  for (; i < n; i++) {
    buffer64[i] = state.x;
    state.x = state.a * state.x & m_mask;
  }
  return VSL_ERROR_OK;
}

/*
  We explicitly stop gcc from vectorizing the following. Since there is no
  64-bit vector multiply instruction in neon, multiplication had to occur in
  scalar register and only after were the results inserted into vector registers
  -- this results in unnecessary overhead.

  Clang does not recognise the attribute optimize("no-tree-vectorize") but it
  also already does not vectorize this function. Hence, we avoid setting any
  specific attributes for it.

  The `% m` operations from the generic implementation have been mostly
  removed since bit shift by 27 combined with cast to uint32 extracts
  the relevant bits already. Only the ones which directly update state.x
  have been kept. This is done to keep consistency with other methods,
  which rely on state.x having had the modulo reduction performed already.
*/
#if defined(__GNUC__) && !defined(__clang__)
__attribute__((optimize("no-tree-vectorize")))
#endif
int Mcg59Generator<CPUFeatures::neon>::fillBits32(int64_t n, uint32_t buffer[]) {
  int i = 0;
  const uint64_t a2 = state.a * state.a;
  const uint64_t a4 = a2 * a2;
  const uint64_t a8 = a4 * a4;
  const uint64_t a12 = a4 * a8;
  const uint64_t a16 = a8 * a8;
  const uint64_t a20 = a16 * a4;
  const uint64_t a24 = a16 * a8;

  for (; i + unroll_count < n; i += unroll_count) {
    const uint64_t x2 = state.x * a2;
    buffer[i + 0] = state.x >> 27;
    buffer[i + 1] = (state.x * state.a) >> 27;
    buffer[i + 2] = x2 >> 27;
    buffer[i + 3] = (x2 * state.a) >> 27;

    buffer[i + 4] = (state.x * a4) >> 27;
    buffer[i + 8] = (state.x * a8) >> 27;
    buffer[i + 12] = (state.x * a12) >> 27;
    buffer[i + 16] = (state.x * a16) >> 27;
    buffer[i + 20] = (state.x * a20) >> 27;

    buffer[i + 5] = (state.x * state.a * a4) >> 27;
    buffer[i + 9] = (state.x * state.a * a8) >> 27;
    buffer[i + 13] = (state.x * state.a * a12) >> 27;
    buffer[i + 17] = (state.x * state.a * a16) >> 27;
    buffer[i + 21] = (state.x * state.a * a20) >> 27;

    buffer[i + 6] = (x2 * a4) >> 27;
    buffer[i + 10] = (x2 * a8) >> 27;
    buffer[i + 14] = (x2 * a12) >> 27;
    buffer[i + 18] = (x2 * a16) >> 27;
    buffer[i + 22] = (x2 * a20) >> 27;

    buffer[i + 7] = (x2 * state.a * a4) >> 27;
    buffer[i + 11] = (x2 * state.a * a8) >> 27;
    buffer[i + 15] = (x2 * state.a * a12) >> 27;
    buffer[i + 19] = (x2 * state.a * a16) >> 27;
    buffer[i + 23] = (x2 * state.a * a20) >> 27;

    state.x = state.x * a24 & m_mask;
  }

  const uint64_t a3 = state.a * a2;
  const uint64_t a5 = a3 * a2;
  const uint64_t a6 = a3 * a3;

  for (; i + 6 < n; i += 6) {
    buffer[i + 0] = state.x >> 27;
    buffer[i + 1] = (state.x * state.a) >> 27;
    buffer[i + 2] = (state.x * a2) >> 27;
    buffer[i + 3] = (state.x * a3) >> 27;
    buffer[i + 4] = (state.x * a4) >> 27;
    buffer[i + 5] = (state.x * a5) >> 27;

    state.x = state.x * a6 & m_mask;
  }

  for (; i < n; i++) {
    buffer[i] = state.x >> 27;
    state.x = state.x * state.a & m_mask;
  }
  return VSL_ERROR_OK;
}

int Mcg59Generator<CPUFeatures::neon>::fill(int64_t n, float buffer[]) {
  int i = 0;
  for (; i + unroll_count < n; i += unroll_count)
    fillArrayUnrolled(buffer + i);

  for (; i < n; i++) {
    buffer[i] = (float)state.x / (float)m;
    state.x = state.a * state.x & m_mask;
  }
  return VSL_ERROR_OK;
}

int Mcg59Generator<CPUFeatures::neon>::fill(int64_t n, double buffer[]) {
  int i = 0;
  for (; i + unroll_count < n; i += unroll_count)
    fillArrayUnrolled(buffer + i);

  for (; i < n; i++) {
    buffer[i] = (double)state.x / (double)m;
    state.x = state.a * state.x & m_mask;
  }
  return VSL_ERROR_OK;
}

namespace {
template <std::floating_point T> inline T transform_to_float(uint64_t x) {
  return (T)x / (T)(1LL << 59);
}
} // namespace

template <std::floating_point T>
void Mcg59Generator<CPUFeatures::neon>::fillArrayUnrolled(T *dst)
  requires(unroll_count == 24)
{

  const uint64_t a2 = state.a * state.a & m_mask;
  const uint64_t a4 = a2 * a2 & m_mask;
  const uint64_t a8 = a4 * a4 & m_mask;
  const uint64_t a12 = a4 * a8 & m_mask;
  const uint64_t a16 = a8 * a8 & m_mask;
  const uint64_t a20 = a16 * a4 & m_mask;
  const uint64_t a24 = a20 * a4 & m_mask;

  const uint64_t x0 = state.x;
  const uint64_t x2 = state.x * a2 & m_mask;

  dst[0] = transform_to_float<T>(x0);
  dst[1] = transform_to_float<T>(x0 * state.a & m_mask);
  dst[2] = transform_to_float<T>(x2);
  dst[3] = transform_to_float<T>(x2 * state.a & m_mask);

  dst[4] = transform_to_float<T>(x0 * a4 & m_mask);
  dst[8] = transform_to_float<T>(x0 * a8 & m_mask);
  dst[12] = transform_to_float<T>(x0 * a12 & m_mask);
  dst[16] = transform_to_float<T>(x0 * a16 & m_mask);
  dst[20] = transform_to_float<T>(x0 * a20 & m_mask);

  dst[5] = transform_to_float<T>(x0 * state.a * a4 & m_mask);
  dst[9] = transform_to_float<T>(x0 * state.a * a8 & m_mask);
  dst[13] = transform_to_float<T>(x0 * state.a * a12 & m_mask);
  dst[17] = transform_to_float<T>(x0 * state.a * a16 & m_mask);
  dst[21] = transform_to_float<T>(x0 * state.a * a20 & m_mask);

  dst[6] = transform_to_float<T>(x2 * a4 & m_mask);
  dst[10] = transform_to_float<T>(x2 * a8 & m_mask);
  dst[14] = transform_to_float<T>(x2 * a12 & m_mask);
  dst[18] = transform_to_float<T>(x2 * a16 & m_mask);
  dst[22] = transform_to_float<T>(x2 * a20 & m_mask);

  dst[7] = transform_to_float<T>(x2 * state.a * a4 & m_mask);
  dst[11] = transform_to_float<T>(x2 * state.a * a8 & m_mask);
  dst[15] = transform_to_float<T>(x2 * state.a * a12 & m_mask);
  dst[19] = transform_to_float<T>(x2 * state.a * a16 & m_mask);
  dst[23] = transform_to_float<T>(x2 * state.a * a20 & m_mask);

  state.x = state.x * a24 & m_mask;
}

int Mcg59Generator<CPUFeatures::neon>::setStride(uint64_t stride) {
  state.a = powMod(state.a, stride, m);

  return VSL_ERROR_OK;
}

int Mcg59Generator<CPUFeatures::neon>::skipAhead(uint64_t n,
                                                 const uint64_t nskip[]) {
  if (n > 1) {
    return VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED;
  }

  auto exponent = nskip[0];
  auto skipA = powMod(state.a, exponent, m);
  state.x = (skipA * state.x) & m_mask;

  return VSL_ERROR_OK;
}

} // namespace openrng
