/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/optimised_kernels.hpp>
#include <utils/cpu_features.hpp>

namespace openrng {

/**
 * Compile time check optimised distribution kernel exists.
 *
 * Similar to the Generator chooser, kernel declarations needs to visible to the
 * choose function in order for IsImplementedKernel to work properly. Typically
 * this means adding them to optimised_kernels.hpp.
 */
template <typename Method, typename... Args>
concept IsImplementedKernel = requires(Args... a) { Method::fill(a...); };

/**
 * Dispatch to optimised distribution kernels.
 */
template <template <CPUFeatures> typename Method, typename... Args>
int choose(Args... args) {
  return Method<CPUFeatures::generic>::fill(args...);
}

} // namespace openrng
