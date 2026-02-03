/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/gaussian_icdf.hpp>
#include <generator/generator.hpp>
#include <openrng.h>
#include <utils/concepts.hpp>
#include <utils/cpu_features.hpp>

#include <cassert>
#include <cmath>

namespace openrng::distribution::gamma {

/**
 * Creates a Gamma sequence using the Gamma GNorm method
 *
 * Gamma GNorm method generates a standard normal variable nv ~ N(0,1) and a
 * uniform variable u ~ U(0,1). It then builds via a shifted-shape constant d
 * and the cube of a scaled normal v, and accepts/rejects with u:
 *
 *   r[i] = d * v
 *
 * The accepted proposal is shifted and scaled to obtain a Gamma-distributed
 * variate with shape alpha and scale beta:
 *
 *   r[i] = a + beta * r[i]
 *
 * The result is stored in r.
 *
 * Assumes n > 0.
 * Assumes generator is a pointer to a valid generator.
 * Assumes r is of length n.
 */

template <CPUFeatures> struct GNorm {};

template <> struct GNorm<CPUFeatures::generic> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[], const T alpha,
                  const T a, const T beta) {
    assert(n > 0);
    assert(generator != nullptr);

    const T d = alpha - static_cast<T>(1) / 3;
    const T c = 1 / std::sqrt(9 * d);

    for (int64_t i = 0; i < n;) {
      T nv, v;
      do {
        gaussian::ICDF<CPUFeatures::generic>::fill(generator, 1, &nv, (T)0,
                                                   (T)1);
        v = static_cast<T>(1) + c * nv;
      } while (v <= 0);
      v = v * v * v;

      // Now generate one uniform number
      T u;
      generator->fill(1, &u);

      if (u < 1 - static_cast<T>(0.0331) * std::pow(nv, 4)) {
        r[i] = d * v;
        ++i;
      } else if (std::log(u) <
                 static_cast<T>(0.5) * nv * nv + d * (1 - v + std::log(v))) {
        r[i] = d * v;
        ++i;
      }
    }

    for (int64_t i = 0; i < n; i++) {
      r[i] = a + beta * r[i];
    }

    return VSL_ERROR_OK;
  }
};

/**
 * This method uses the Ahrens and Dieter algorirthm to generate the numbers.
 * That method is mentioned and shown in the Devroye 1986 paper titled
 * "Non-Uniform Random Variate Generation" in Chapter 9, section 3, page 425:
 * https://luc.devroye.org/chapter_nine.pdf
 */
template <CPUFeatures> struct GNorm_AD {};

template <> struct GNorm_AD<CPUFeatures::generic> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[], const T alpha,
                  const T a, const T beta) {
    assert(n > 0);
    assert(generator != nullptr);

    constexpr T e = static_cast<T>(0x1.5bf0a8b145769p1);
    const T b = (e + alpha) / e;
    const T c = 1 / alpha;
    T v, u[2];

    for (int64_t i = 0; i < n;) {
      generator->fill(2, u);
      v = b * u[0];

      if (v <= 1.0) {
        r[i] = std::pow(v, c);
        if (u[1] <= std::pow(e, -r[i])) {
          ++i;
        }
        continue;
      }
      r[i] = -std::log(c * (b - v));
      if (u[1] <= std::pow(r[i], alpha - 1)) {
        ++i;
      }
    }

    for (int64_t i = 0; i < n; i++) {
      r[i] = a + beta * r[i];
    }

    return VSL_ERROR_OK;
  }
};

/** This algorithm is using the rejection method from Weibull based on the
 * algorithm mentioned in the paper by Vaduva, I., titled: "On computer
 * generation of gamma random variables by rejection and composition
 * procedures". This algorithm is slightly changed in Devroye's 1986 paper based
 * on the Gamma density and it uses an exponential number, which when put
 * through some calculus is a Weibull distribution of alpha at some point, see
 * chapter 9: https://luc.devroye.org/chapter_nine.pdf page 415.
 *
 * In Devroye's book, on page 415, in chapter 9, there is a typo in the
 * algorithm he proposes. It says that the acceptance condition should be
 * expo[0] + expo[1] <= d + r[i], when it should actually say "UNTIL Z + E > d +
 *X", therefore the condition becomes expo[0] + expo[1] > d + r[i]. This paper
 *confirms the typo made in his book:
 *https://numerical.recipes/devroye/Devroye_files/errors.pdf
 **/
template <CPUFeatures> struct GNorm_RW {};

template <> struct GNorm_RW<CPUFeatures::generic> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[], const T alpha,
                  const T a, const T beta) {
    assert(n > 0);
    assert(generator != nullptr);

    const T c = 1 / alpha;
    const T d = std::pow(alpha, alpha / (1 - alpha)) * (1 - alpha);
    T expo[2];

    for (int64_t i = 0; i < n;) {
      exponential::ICDF<CPUFeatures::generic>::fill(generator, 2, expo, T(0),
                                                    T(1));
      r[i] = std::pow(expo[0], c);
      if (expo[0] + expo[1] > d + r[i]) {
        ++i;
      }
    }

    for (int64_t i = 0; i < n; i++) {
      r[i] = a + beta * r[i];
    }

    return VSL_ERROR_OK;
  }
};

} // namespace openrng::distribution::gamma