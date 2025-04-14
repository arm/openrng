/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/optimised_generators.hpp>
#include <utils/cpu_features.hpp>
#include <utils/expected.hpp>

#include <type_traits>

namespace openrng {

/* We assume that, where a generator has been optimised for a specific CPU
   feature, there is a class templated on CPUFeatures for that generator. There
   should exist specialisations of the class for the CPUFeatures that it is
   optimised for, which should derive from Generator, and the unspecialised
   class should be empty (note it should still be complete - the terminology is
   slightly overloaded here). This enables the choosing mechanism to only try
   and choose optimised generators which actually exist. The declaration of the
   specialised class needs to be visible to the choose function in order for
   this to work - typically this means adding the declaration to
   optimised_generators.hpp.  */

template <typename T>
concept IsCompleteGenerator = std::is_base_of<Generator, T>::value;

template <template <CPUFeatures> class DerivedGenerator, typename... Args>
Expected<Generator> Generator::choose(Args &&...args) {
  /* Compile-time check of IsCompleteGenerator prevents calls to empty
     (non-existent) optimised generators being emitted.  */
  if constexpr (IsCompleteGenerator<DerivedGenerator<CPUFeatures::sha3>>)
    if (SupportedFeatures::hasSha3)
      return create<DerivedGenerator<CPUFeatures::sha3>>(args...);

  if constexpr (IsCompleteGenerator<DerivedGenerator<CPUFeatures::aes>>)
    if (SupportedFeatures::hasAes)
      return create<DerivedGenerator<CPUFeatures::aes>>(args...);

  if constexpr (IsCompleteGenerator<DerivedGenerator<CPUFeatures::neon>>)
    if (SupportedFeatures::hasNeon)
      return create<DerivedGenerator<CPUFeatures::neon>>(args...);

  if constexpr (IsCompleteGenerator<DerivedGenerator<CPUFeatures::trng>>)
    if (SupportedFeatures::hasTrng)
      return create<DerivedGenerator<CPUFeatures::trng>>(args...);

  return create<DerivedGenerator<CPUFeatures::generic>>(args...);
}

} // namespace openrng
