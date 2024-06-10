/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/generator.hpp>
#include <generator/mcg31m1.hpp>
#include <generator/mcg59.hpp>
#include <generator/mrg32k3a.hpp>
#include <generator/mt19937.hpp>
#include <generator/nondeterministic.hpp>
#include <generator/optimised_generators.hpp>
#include <generator/philox4x3210.hpp>
#include <generator/r250.hpp>
#include <generator/sfmt19937.hpp>
#include <generator/sobol.hpp>
#include <openrng.h>
#include <utils/choose_generator.hpp>
#include <utils/expected.hpp>

namespace openrng {

template <typename... Args>
Expected<Generator> Generator::create(const int64_t brng, Args &&...args) {

  switch (brng) {
  case VSL_BRNG_MCG31:
    return Generator::choose<Mcg31m1Generator>(args...);
  case VSL_BRNG_MRG32K3A:
    return Generator::choose<MRG32k3aGenerator>(args...);
  case VSL_BRNG_MT19937:
    return Generator::choose<MT19937Generator>(args...);
  case VSL_BRNG_R250:
    return Generator::create<R250Generator>(args...);
  case VSL_BRNG_MCG59:
    return Generator::choose<Mcg59Generator>(args...);
  case VSL_BRNG_WH:
    return {nullptr, VSL_ERROR_FEATURE_NOT_IMPLEMENTED};
  case VSL_BRNG_SOBOL:
    return Generator::create<SobolGenerator>(args...);
  case VSL_BRNG_NIEDERR:
    return {nullptr, VSL_ERROR_FEATURE_NOT_IMPLEMENTED};
  case VSL_BRNG_MT2203:
    return {nullptr, VSL_ERROR_FEATURE_NOT_IMPLEMENTED};
  case VSL_BRNG_IABSTRACT:
    return {nullptr, VSL_ERROR_FEATURE_NOT_IMPLEMENTED};
  case VSL_BRNG_DABSTRACT:
    return {nullptr, VSL_ERROR_FEATURE_NOT_IMPLEMENTED};
  case VSL_BRNG_SABSTRACT:
    return {nullptr, VSL_ERROR_FEATURE_NOT_IMPLEMENTED};
  case VSL_BRNG_SFMT19937:
    return Generator::choose<SFMT19937Generator>(args...);
  case VSL_BRNG_NONDETERM:
    return Generator::choose<NondeterministicGenerator>(args...);
  case VSL_BRNG_ARS5:
    return {nullptr, VSL_ERROR_FEATURE_NOT_IMPLEMENTED};
  case VSL_BRNG_PHILOX4X32X10:
    return Generator::choose<Philox4x3210Generator>(args...);
  default:
    return {nullptr, VSL_RNG_ERROR_INVALID_BRNG_INDEX};
  }
}

template Expected<Generator> Generator::create(int64_t, int64_t &&,
                                               const unsigned int *&);

template Expected<Generator> Generator::create(int64_t, Generator *&);

template Expected<Generator> Generator::create(int64_t,
                                               const StateWrapperBase *&);

} // namespace openrng
