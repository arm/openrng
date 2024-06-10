/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <concepts>

namespace openrng {

// Internal floating point concept. Currently VSL only supports float and
// double, so no need to write code that supports all floating point types.
template <typename T>
concept FloatingPoint = std::same_as<float, T> || std::same_as<double, T>;

} // namespace openrng
