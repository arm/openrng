/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/neon/sfmt19937_neon_impl.hpp>

namespace openrng {
template class SFMT19937Generator<CPUFeatures::sha3>;
} // namespace openrng
