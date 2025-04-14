/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <bit>

namespace openrng::math {
namespace {

template <typename ToType, typename FromType> inline ToType as(FromType x) {
  return std::bit_cast<ToType>(x);
}

template <int N, typename T> inline T horner(T x, const T poly[]) {
  auto p = poly[N - 1] + poly[N] * x;
  for (int i = N - 2; i >= 0; i--) {
    p = poly[i] + x * p;
  }
  return p;
}

} // namespace
} // namespace openrng::math
