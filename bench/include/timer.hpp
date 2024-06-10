/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <chrono>

namespace openrng::bench {

static inline std::chrono::time_point<std::chrono::steady_clock> timer_start() {
  return std::chrono::steady_clock::now();
}

static inline double
timer_end(std::chrono::time_point<std::chrono::steady_clock> start) {
  auto end = std::chrono::steady_clock::now();
  const std::chrono::duration<double> diff = end - start;
  return diff.count();
}

}; // namespace openrng::bench
