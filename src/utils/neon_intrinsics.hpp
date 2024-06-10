/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

/* Mini-ACLE for common vector operations which do not appear in the ACLE.  */

#pragma once

#include <arm_neon.h>

namespace openrng {

inline uint64x2_t vmull_low_u32(uint32x4_t x, uint32x4_t y) {
  return vmull_u32(vget_low_u32(x), vget_low_u32(y));
}

} // namespace openrng
