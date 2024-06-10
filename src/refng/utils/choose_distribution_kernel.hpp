/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <utils/cpu_features.hpp>

namespace refng {

/**
 * Dispatch to optimised distribution kernels - for refng just return generic.
 */
template <template <CPUFeatures> typename Method, typename... Args>
int choose(Args... args) {
  return Method<CPUFeatures::generic>::fill(args...);
}

} // namespace refng
