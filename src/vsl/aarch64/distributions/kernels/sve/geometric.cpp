/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <distributions/kernels/sve/geometric.hpp>

#include <math/log.hpp>

namespace openrng::distribution::geometric {

int ICDF<CPUFeatures::sve>::fill(Generator *generator, const int64_t n,
                                 int32_t r[], const double p) {
  assert(n > 0);
  assert(generator != nullptr);

  static_assert(sizeof(*r) == sizeof(float));
  auto *floatView = reinterpret_cast<float *>(r);

  const double g = 1. / math::log(1. - p);

  generator->fill(n, floatView);
  for (int i = 0; i < n; i++) {
    r[i] = math::log(floatView[i]) * g;
  }

  return VSL_ERROR_OK;
}

} // namespace openrng::distribution::geometric
