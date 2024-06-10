/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

namespace openrng {

/**
 * Inspired by std::expected in c++23. If error is VSL_ERROR_OK, then
 * value is a valid pointer, otherwise it is invalid.
 */
template <typename T> struct Expected {
  T *value;
  int error;
};

} // namespace openrng