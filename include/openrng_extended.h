/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup groupService
 *
 * This function returns the current major, minor, and patch version of OpenRNG.
 * For instance, for OpenRNG 25.04.1, the major version is 25, the minor version
 * is 4 and, the patch version is 1. If any of the three elements is not set,
 * then it is set to -1 as default. For example, for release 24.10 the returned
 * patch version would be -1.
 *
 * \param[out] major The 32-bit integer representing the current major version.
 * \param[out] minor The 32-bit integer representing the current minor version.
 * \param[out] patch The 32-bit integer representing the current patch version.
 */
void openRngGetVersion(int32_t *major, int32_t *minor, int32_t *patch);

#ifdef __cplusplus
}
#endif
