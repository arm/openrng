/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/generator.hpp>
#include <utils/cpu_features.hpp>
#include <utils/expected.hpp>

#include <type_traits>

namespace openrng {

template <template <CPUFeatures> class DerivedGenerator, typename... Args>
Expected<Generator> Generator::choose(Args &&...args) {
  return Generator::create<DerivedGenerator<CPUFeatures::generic>>(args...);
}

} // namespace openrng
