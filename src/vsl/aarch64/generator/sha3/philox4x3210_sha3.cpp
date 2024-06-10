/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/neon/philox4x3210_neon_impl.hpp>

namespace openrng {
template class Philox4x3210Generator<CPUFeatures::sha3>;
}
