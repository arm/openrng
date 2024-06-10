/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <distributions/kernels/sve/exponential.hpp>

#include <math/log.hpp>

namespace openrng::distribution::exponential {

template <openrng::FloatingPoint T>
int ICDF<CPUFeatures::sve>::fill(Generator *generator, const int64_t n, T r[],
                                 const T a, const T beta) {
  assert(n > 0);
  assert(generator != nullptr);

  generator->fill(n, r);

  for (int64_t i = 0; i < n; i++) {
    r[i] = -beta * math::log(r[i]) + a;
  }

  return VSL_ERROR_OK;
}

template int ICDF<CPUFeatures::sve>::fill(Generator *, const int64_t, float[],
                                          const float, const float);
template int ICDF<CPUFeatures::sve>::fill(Generator *, const int64_t, double[],
                                          const double, const double);

} // namespace openrng::distribution::exponential
