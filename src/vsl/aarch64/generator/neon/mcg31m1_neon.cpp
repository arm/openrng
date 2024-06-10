/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/neon/mcg31m1_neon.hpp>
#include <utils/helpers.hpp>

namespace openrng {

int Mcg31m1Generator<CPUFeatures::neon>::initialise(
    const int64_t n, const unsigned int params[]) {
  unsigned int seed = 0;
  if (n == 0) {
    seed = 1;
  } else {
    seed = params[0];
  }

  state.x = seed % m;

  if (state.x == 0) {
    state.x = 1;
  }

  return VSL_ERROR_OK;
}

int Mcg31m1Generator<CPUFeatures::neon>::fillBits(int64_t n,
                                                  uint32_t buffer[]) {
  int i = 0;
  const uint64_t a2 = state.a * state.a % m;
  const uint64_t a4 = a2 * a2 % m;
  const uint64_t a8 = a4 * a4 % m;
  const uint64_t a12 = a4 * a8 % m;
  const uint64_t a16 = a8 * a8 % m;

  for (; i + unroll_count < n; i += unroll_count) {
    const uint32_t x2 = state.x * a2 % m;
    buffer[i + 0] = state.x;
    buffer[i + 1] = state.x * state.a % m;
    buffer[i + 2] = x2;
    buffer[i + 3] = x2 * state.a % m;

    for (int j = 0; j < 4; j++) {
      buffer[i + 4 + j] = buffer[i + j] * a4 % m;
      buffer[i + 8 + j] = buffer[i + j] * a8 % m;
      buffer[i + 12 + j] = buffer[i + j] * a12 % m;
      buffer[i + 16 + j] = buffer[i + j] * a16 % m;
    }

    state.x = buffer[i + unroll_count - 1] * state.a % m;
  }

  const uint64_t a3 = state.a * a2 % m;

  for (; i + 4 < n; i += 4) {
    buffer[i + 0] = state.x;
    buffer[i + 1] = state.a * state.x % m;
    buffer[i + 2] = a2 * state.x % m;
    buffer[i + 3] = a3 * state.x % m;
    state.x = a4 * state.x % m;
  }

  for (; i < n; i++) {
    buffer[i] = state.x;
    state.x = state.a * state.x % m;
  }
  return VSL_ERROR_OK;
}

namespace {
template <std::floating_point T> inline T transform_to_float(uint32_t x) {
  return (T)x / (T)0x7fffffff;
}
} // namespace

template <std::floating_point T>
void Mcg31m1Generator<CPUFeatures::neon>::fillArrayUnrolled(T *dst,
                                                            const T scale,
                                                            const T l)
  requires(unroll_count == 20)
{

  const uint64_t a2 = state.a * state.a % m;

  const uint32_t x0 = state.x;
  const uint32_t x1 = state.x * state.a % m;
  const uint32_t x2 = state.x * a2 % m;
  const uint32_t x3 = x2 * state.a % m;

  dst[0] = (transform_to_float<T>(x0)) * scale + l;
  dst[1] = (transform_to_float<T>(x1)) * scale + l;
  dst[2] = (transform_to_float<T>(x2)) * scale + l;
  dst[3] = (transform_to_float<T>(x3)) * scale + l;

  const uint64_t a4 = a2 * a2 % m;
  const uint64_t a8 = a4 * a4 % m;
  const uint64_t a12 = a4 * a8 % m;
  const uint64_t a16 = a8 * a8 % m;

  dst[4] = (transform_to_float<T>(x0 * a4 % m)) * scale + l;
  dst[8] = (transform_to_float<T>(x0 * a8 % m)) * scale + l;
  dst[12] = (transform_to_float<T>(x0 * a12 % m)) * scale + l;
  dst[16] = (transform_to_float<T>(x0 * a16 % m)) * scale + l;

  dst[5] = (transform_to_float<T>(x1 * a4 % m)) * scale + l;
  dst[9] = (transform_to_float<T>(x1 * a8 % m)) * scale + l;
  dst[13] = (transform_to_float<T>(x1 * a12 % m)) * scale + l;
  dst[17] = (transform_to_float<T>(x1 * a16 % m)) * scale + l;

  dst[6] = (transform_to_float<T>(x2 * a4 % m)) * scale + l;
  dst[10] = (transform_to_float<T>(x2 * a8 % m)) * scale + l;
  dst[14] = (transform_to_float<T>(x2 * a12 % m)) * scale + l;
  dst[18] = (transform_to_float<T>(x2 * a16 % m)) * scale + l;

  const uint32_t x19 = x3 * a16 % m;
  dst[7] = (transform_to_float<T>(x3 * a4 % m)) * scale + l;
  dst[11] = (transform_to_float<T>(x3 * a8 % m)) * scale + l;
  dst[15] = (transform_to_float<T>(x3 * a12 % m)) * scale + l;
  dst[19] = (transform_to_float<T>(x19)) * scale + l;

  state.x = x19 * state.a % m;
}

int Mcg31m1Generator<CPUFeatures::neon>::fill(int64_t n, float buffer[]) {
  return fill(n, buffer, 0.0f, 1.0f);
}

int Mcg31m1Generator<CPUFeatures::neon>::fill(int64_t n, double buffer[]) {
  return fill(n, buffer, 0.0, 1.0);
}

int Mcg31m1Generator<CPUFeatures::neon>::fill(int64_t n, float buffer[],
                                              const float l, const float u) {
  int i = 0;
  const float scale = u - l;
  for (; i + 20 < n; i += 20)
    fillArrayUnrolled(buffer + i, scale, l);

  for (; i < n; i++) {
    buffer[i] = ((float)state.x / (float)m) * scale + l;
    state.x = state.a * state.x % m;
  }
  return VSL_ERROR_OK;
}

int Mcg31m1Generator<CPUFeatures::neon>::fill(int64_t n, double buffer[],
                                              const double l, const double u) {
  int i = 0;
  const double scale = u - l;
  for (; i + 20 < n; i += 20)
    fillArrayUnrolled(buffer + i, scale, l);

  for (; i < n; i++) {
    buffer[i] = ((double)state.x / (double)m) * scale + l;
    state.x = state.a * state.x % m;
  }
  return VSL_ERROR_OK;
}

int Mcg31m1Generator<CPUFeatures::neon>::setStride(uint64_t stride) {
  state.a = powMod(state.a, stride, m);

  return VSL_ERROR_OK;
}

int Mcg31m1Generator<CPUFeatures::neon>::skipAhead(uint64_t n,
                                                   const uint64_t nskip[]) {
  if (n > 1) {
    return VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED;
  }

  auto exponent = nskip[0];
  auto skipA = powMod(state.a, exponent, m);
  state.x = (skipA * state.x) % m;

  return VSL_ERROR_OK;
}

int Mcg31m1Generator<CPUFeatures::neon>::fillBits32(int64_t, uint32_t[]) {
  return VSL_RNG_ERROR_BRNG_NOT_SUPPORTED;
}

int Mcg31m1Generator<CPUFeatures::neon>::fillBits64(int64_t, uint64_t[]) {
  return VSL_RNG_ERROR_BRNG_NOT_SUPPORTED;
}

} // namespace openrng
