/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <distributions/kernels/exponential.hpp>
#include <distributions/kernels/gamma.hpp>
#include <distributions/kernels/neon/gaussian_icdf.hpp>

namespace openrng::distribution::gamma {
template <> struct GNorm<CPUFeatures::neon> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[], const T alpha,
                  const T a, const T beta) {
    assert(n > 0);
    assert(generator != nullptr);

    const T d = alpha - static_cast<T>(1) / 3;
    const T c = 1 / std::sqrt(9 * d);
    DynamicArray<T> gaussians(n);
    DynamicArray<T> uniforms(n);
    int64_t gaussianIndex = n;
    int64_t uniformIndex = n;

    for (int64_t i = 0; i < n;) {
      if (gaussianIndex == n) {
        gaussian::ICDF<CPUFeatures::neon>::fill(
            generator, n - i, gaussians.data() + i, (T)0, (T)1);
        gaussianIndex = i;
      }

      const T nv = gaussians[gaussianIndex++];
      T v = static_cast<T>(1) + c * nv;
      if (v <= 0) {
        continue;
      }

      v = v * v * v;

      if (uniformIndex == n) {
        generator->fill(n - i, uniforms.data() + i);
        uniformIndex = i;
      }

      const T u = uniforms[uniformIndex++];

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

template <> struct GNorm_AD<CPUFeatures::neon> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[], const T alpha,
                  const T a, const T beta) {
    assert(n > 0);
    assert(generator != nullptr);

    constexpr T e = static_cast<T>(0x1.5bf0a8b145769p1);
    const T b = (e + alpha) / e;
    const T c = T(1) / alpha;

    int64_t kBlock = 2 * n;
    DynamicArray<T> uniforms(kBlock, T(-1));
    int64_t count = kBlock;

    auto refill = [&]() {
      generator->fill(kBlock, uniforms.data());
      count = 0;
    };

    int64_t i = 0;
    while (i < n) {
      if (count + 1 >= kBlock) {
        kBlock = 2 * (n - i);
        refill();
      }

      const T u0 = uniforms[count];
      const T u1 = uniforms[count + 1];
      count += 2;

      const T v = b * u0;

      if (v <= T(1)) {
        const T ri = std::pow(v, c);
        if (u1 <= std::pow(e, -ri)) {
          r[i++] = ri;
        }
      } else {
        const T ri = -math::log(c * (b - v));
        if (u1 <= std::pow(ri, alpha - T(1))) {
          r[i++] = ri;
        }
      }
    }

    for (int64_t j = 0; j < n; ++j) {
      r[j] = a + beta * r[j];
    }

    return VSL_ERROR_OK;
  }
};

template <> struct GNorm_RW<CPUFeatures::neon> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[], const T alpha,
                  const T a, const T beta) {
    assert(n > 0);
    assert(generator != nullptr);

    const T c = T(1) / alpha;
    const T d = std::pow(alpha, alpha / (1 - alpha)) * (1 - alpha);

    int64_t kBlock = 2 * n;
    DynamicArray<T> expo_vect(kBlock, T(-1));
    int64_t expo_index = kBlock;

    auto refill = [&]() {
      exponential::ICDF<CPUFeatures::neon>::fill(generator, kBlock,
                                                 expo_vect.data(), T(0), T(1));
      expo_index = 0;
    };

    int64_t i = 0;
    while (i < n) {
      if (expo_index + 1 >= kBlock) {
        kBlock = 2 * (n - i);
        refill();
      }

      const T e0 = expo_vect[expo_index];
      const T e1 = expo_vect[expo_index + 1];
      expo_index += 2;

      r[i] = std::pow(e0, c);
      if (e0 + e1 > d + r[i]) {
        i++;
      }
    }

    for (int64_t j = 0; j < n; ++j) {
      r[j] = a + beta * r[j];
    }

    return VSL_ERROR_OK;
  }
};
} // namespace openrng::distribution::gamma