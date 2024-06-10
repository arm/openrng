/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <cstring>
#include <generator/philox4x3210.hpp>
#include <utils/philox.hpp>

namespace openrng {

namespace {

void write_elems(uint32_t *dst, const uint32_t *src, size_t n) {
  memcpy(dst, src, n * sizeof(uint32_t));
}

template <std::floating_point T>
void write_elems(T *dst, const uint32_t *src, size_t n) {
  for (unsigned i = 0; i < n; i++)
    dst[i] = (int)src[i] / 0x1p32 + 0.5;
}

} // namespace

int Philox4x3210Generator<CPUFeatures::generic>::initialise(
    const int64_t n, const unsigned int params[]) {
  for (int i = 0; i < 4; i++)
    state.c.data.u32[i] = n > i + 2 ? params[i + 2] : 0;

  state.k0 = n == 0 ? 0 : params[0];
  state.k1 = n > 1 ? params[1] : 0;

  state.off = 0;

  return VSL_ERROR_OK;
}

void Philox4x3210Generator<CPUFeatures::generic>::advance(uint64_t by) {
  /* Every 4 advances, c is incremented. If by is a multiple of 4 then we can
   * just bump c and be done. If not, bump c by the number of full quadwords
   * that this advance would skip over, and adjust offset such that when we
   * write to the array we begin from the correct subelement of philox(c).  */
  state.c.data.u128 += (state.off + by) / 4;
  state.off = (state.off + by) % 4;
}

uint128_t Philox4x3210Generator<CPUFeatures::generic>::get_philox() const {
  return philox_10(state.k0, state.k1, state.c);
}

template <typename T>
void Philox4x3210Generator<CPUFeatures::generic>::fill_array(int64_t n,
                                                             T *dst) {
  /* Walking over the array has at most 3 stages: head, body and tail.  */
  int64_t i = 0;
  uint128_t c = get_philox();

  /* Head: exhaust the current value of c. Starting from current offset into
   * c, write either the remainder of the quadword or n elements (whichever is
   * less). If offset is already zero then this will be a no-op.  */
  const int64_t chunksize = std::min(static_cast<int64_t>(4) - state.off, n);
  write_elems(dst, c.data.u32 + state.off, chunksize);
  advance(chunksize);
  i += chunksize;

  /* Body: while there are at least 4 remaining elements to write, deal with the
   * entire quadword. Destination pointer is offset by the length of the head,
   * and offset is 0 since the previous quad was exhausted when writing the
   * head.  */
  for (; i + 4 < n; i += 4) {
    c = get_philox();
    write_elems(dst + i, c.data.u32, 4);
    advance(4);
  }

  /* Tail: if there are between 1 and 3 elements left to write. advance will
   * update offset but not bump c so that subsequent takes will use the current
   * value of c, but index into it from further on.  */
  if (i < n) {
    c = get_philox();
    const int64_t ntail = n - i;
    write_elems(dst + i, c.data.u32, ntail);
    advance(ntail);
  }
}

int Philox4x3210Generator<CPUFeatures::generic>::fillBits(int64_t n,
                                                          uint32_t dst[]) {
  fill_array(n, dst);
  return VSL_ERROR_OK;
}

int Philox4x3210Generator<CPUFeatures::generic>::fillBits32(int64_t n,
                                                            uint32_t dst[]) {
  return fillBits(n, dst);
}

int Philox4x3210Generator<CPUFeatures::generic>::fillBits64(int64_t n,
                                                            uint64_t dst[]) {
  return fillBits(n * 2, (uint32_t *)dst);
}

int Philox4x3210Generator<CPUFeatures::generic>::fill(int64_t n, float dst[]) {
  fill_array(n, dst);
  return VSL_ERROR_OK;
}

int Philox4x3210Generator<CPUFeatures::generic>::fill(int64_t n, double dst[]) {
  fill_array(n, dst);
  return VSL_ERROR_OK;
}

int Philox4x3210Generator<CPUFeatures::generic>::skipAhead(
    uint64_t n, const uint64_t nskip[]) {
  /* Period of Philox4x32 is 2^130 (get 4 words for each value of the 128-bit
   * counter, so all but the first 3 elements of nskip can be ignored.  */

  /* 2^0 part.  */
  advance(nskip[0]);

  /* 2^64 part - no need to mess with offset as j * 2^64 is a multiple of 4.
   * Just bump c by j * 2^62.  */
  if (n > 1)
    state.c.data.u128 += ((__uint128_t)nskip[1]) << 62;

  /* 2^128 part - we only care about first 2 bits, since any values > 4 mean
   * advancing by a multiple of 2^130, which is circular. Similar to 2^64 part,
   * advancing by j * 2^128 means bumping c by j * 2^126.  */
  if (n > 2)
    state.c.data.u128 += ((__uint128_t)nskip[2]) << 126;

  return VSL_ERROR_OK;
}

int Philox4x3210Generator<CPUFeatures::generic>::setStride(uint64_t) {
  return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
}

} // namespace openrng
