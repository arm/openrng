/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <math/log_data.hpp>
#include <math/util.hpp>
#include <utils/concepts.hpp>

#include <cstdint>
#include <numbers>

namespace openrng::math {
namespace {

inline double log(double x) {
  using d = log_data;
  constexpr auto A = d::A;
  constexpr auto lookup = [](auto i) {
    constexpr auto mask = (1 << d::table_bits) - 1;
    const auto idx = (i >> (52 - d::table_bits)) & mask;
    return d::table[idx];
  };

  const auto ix = as<uint64_t>(x);

  const auto tmp = ix - d::off;
  const auto k = (int64_t)tmp >> 52;
  const auto iz = ix - (tmp & d::sign_exp_mask);
  const auto z = as<double>(iz);
  const auto e = lookup(tmp);

  const auto r = z * e.invc - 1.0;
  const auto kd = (double)k;

  const auto hi = (e.logc + r) + kd * std::numbers::ln2;

  const auto r2 = r * r;
  auto y = A(2) + A(3) * r;
  auto p = A(0) + A(1) * r;
  y += A(4) * r2;
  y = p + y * r2;

  return hi + y * r2;
}

inline float log(float x) {
  using d = logf_data;
  constexpr auto P = d::P;

  auto u = as<uint32_t>(x) - d::off;

  const float n = (int32_t)u >> 23;
  u &= 0x007fffff;
  u += d::off;

  const auto r = as<float>(u) - 1.0f;
  const auto r2 = r * r;

  auto p = P(5) + P(6) * r;
  auto q = P(3) + P(4) * r;
  auto y = P(1) + P(2) * r;
  p += P(7) * r2;
  q += p * r2;
  y += q * r2;
  p = r + std::numbers::ln2_v<float> * n;
  return (p + y * r2);
}

} // namespace
} // namespace openrng::math
