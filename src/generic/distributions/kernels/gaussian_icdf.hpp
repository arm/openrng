/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/concepts.hpp>
#include <utils/cpu_features.hpp>
#include <utils/erfinv.hpp>

#include <cassert>
#include <cmath>

namespace openrng::distribution::gaussian {

template <CPUFeatures> struct ICDF {};

/**
 * Creates a Gaussian sequence using the ICDF method.
 *
 * The ICDF method applies the following transformation to a variable, uniformly
 * distributed on the range (-1, 1), to generate a normally distributed
 * variable.
 *
 *   x = sqrt(2) * erfinv(u)
 *
 * where erfinv is the inverse error function. A normally distributed variable
 * can be transformed into a gaussian variable using
 *
 *   y = s * x + a
 *
 * Assumes:
 *  * n > 0.
 *  * generator is a pointer to a valid generator.
 *  * r is of length n.
 */
template <> struct ICDF<CPUFeatures::generic> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[], const T a,
                  const T sigma) {
    assert(n > 0);
    assert(generator != nullptr);

    generator->fill(n, r);

    for (int64_t i = 0; i < n; i++) {
      T u = 2 * r[i] - 1;

      // erfinv has singularities at +/- 1, that is, it returns +/-inf
      // respectively for those inputs. We avoid the singularities by perturbing
      // the inputs.
      if (u == -1 || u == 1) {
        u = std::nextafter(T{u}, T{0});
      }

      r[i] = sigma * sqrt(2) * erfinv(u) + a;
    }

    return VSL_ERROR_OK;
  }
};
} // namespace openrng::distribution::gaussian
