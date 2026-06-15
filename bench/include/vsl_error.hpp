/*
 * SPDX-FileCopyrightText: <text>Copyright 2024, 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <stdexcept>
#include <string>

#include <openrng.h>

namespace openrng::bench {

inline void check_no_error(int err) {
  if (err != VSL_ERROR_OK)
    throw std::runtime_error("VSL error: " + std::to_string(err));
}

} // namespace openrng::bench
