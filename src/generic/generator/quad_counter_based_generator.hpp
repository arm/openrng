/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <cstring>
#include <generator/copyable_generator.hpp>
#include <utils/uint128.hpp>

namespace openrng {

template <typename Key> struct QuadCounterBasedState {
  // Counter state.
  uint128_t c = 0;
  // Encryption keys specified by the type `Key`
  // They cannot be marked const because they are set at initialisation, but
  // they should not be changed afterwards.
  Key k = {};
  // Offset used when filling array begins in the middle of a value of c.
  uint32_t off = 0;
};

namespace {

inline void write_elems(uint32_t *dst, const uint32_t *src, size_t n) {
  memcpy(dst, src, n * sizeof(uint32_t));
}

template <std::floating_point T>
void write_elems(T *dst, const uint32_t *src, size_t n) {
  for (unsigned i = 0; i < n; ++i)
    dst[i] = (int)src[i] / 0x1p32 + 0.5;
}

} // namespace

template <CPUFeatures, typename> class QuadCounterBasedGenerator {};

template <typename Key>
class QuadCounterBasedGenerator<CPUFeatures::generic, Key>
    : public CopyableGenerator<QuadCounterBasedState<Key>> {

private:
  inline void advance(uint64_t by) {
    // Increment c every 4 advances.
    this->state.c += (this->state.off + by) / 4;
    // Adjust offset of the next subelement to be outputted.
    this->state.off = (this->state.off + by) % 4;
  }

protected:
  template <typename T,
            uint128_t (*do_bijection)(const QuadCounterBasedState<Key> &)>
  void fill_array(int64_t n, T *dst) {
    int64_t i = 0;

    // Exhaust remaining elements
    uint128_t c = do_bijection(this->state);
    const int64_t nhead =
        std::min(static_cast<int64_t>(4) - this->state.off, n);
    write_elems(dst, c.data.u32 + this->state.off, nhead);
    advance(nhead);
    i += nhead;

    // Process 4 elements as a quadword at a time
    for (; i + 4 < n; i += 4) {
      c = do_bijection(this->state);
      write_elems(dst + i, c.data.u32, 4);
      advance(4);
    }

    // Handle remaining elements left
    if (i < n) {
      c = do_bijection(this->state);
      const int64_t ntail = n - i;
      write_elems(dst + i, c.data.u32, ntail);
      advance(ntail);
    }
  }

public:
  QuadCounterBasedGenerator(int64_t _brngId)
      : CopyableGenerator<QuadCounterBasedState<Key>>{_brngId} {}

  int setStride(uint64_t) final { return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED; }

  int skipAhead(uint64_t n, const uint64_t nskip[]) final {
    // Period is 2^130, so all but the first 3 elements of nskip can be ignored.

    // 2^0 part
    advance(nskip[0]);

    // 2^64 part - no need to mess with offset as j * 2^64 is a multiple of 4.
    if (n > 1)
      this->state.c += uint128_t{nskip[1]} << 62;

    // 2^128 part - we only care about first 2 bits, since any values > 4 mean
    // advancing by a multiple of 2^130, which is circular.
    if (n > 2)
      this->state.c += uint128_t{nskip[2]} << 126;

    return VSL_ERROR_OK;
  }
};

} // namespace openrng
