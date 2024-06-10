/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <math/log_data.hpp>

#include <cstdint>

namespace openrng::math::sve {
namespace {

template <typename T> struct log_data;

template <> struct log_data<float> {
  constexpr static auto p0 = -0x1.ffffc8p-2f, p1 = 0x1.555d7cp-2f,
                        p2 = -0x1.00187cp-2f, p3 = 0x1.961348p-3f,
                        p4 = -0x1.4f9934p-3f, p5 = 0x1.5a9aa2p-3f,
                        p6 = -0x1.3e737cp-3f;
  constexpr static float p1356[] = {p1, p3, p5, p6};
  constexpr static uint32_t off = 0x3f2aaaab, mantissa_mask = 0x007fffff;
};

template <> struct log_data<double> {
  constexpr static auto table = math::log_data::table;
  constexpr static auto table_bits = math::log_data::table_bits;
  constexpr static double p[] = {-0x1.ffffffffcbad3p-2, 0x1.555555578ed68p-2,
                                 -0x1.0000d3a1e7055p-2, 0x1.999392d02a63ep-3};
  constexpr static uint64_t sign_exp_mask = 0xfff0000000000000;
  constexpr static uint64_t off = 0x3fe6900900000000;
};

} // namespace
} // namespace openrng::math::sve
