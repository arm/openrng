/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

inline void getLibraryVersion(int32_t *major, int32_t *minor, int32_t *patch) {
  *major = OPENRNG_MAJOR;
  *minor = OPENRNG_MINOR;
  *patch = OPENRNG_PATCH;
}
