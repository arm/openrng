/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <distributions/kernels/gaussian_icdf.hpp>
#include <math/sve/erfinv.hpp>

namespace openrng::distribution::gaussian {

namespace {
template <openrng::FloatingPoint T>
inline void icdf(int64_t n, T *r, T a, T sigma);

template <> inline void icdf(int64_t n, float *r, float a, float sigma) {
  for (int64_t i = 0; i < n; i += svcntw()) {
    const auto pg = svwhilelt_b32(i, n);
    auto u = svld1(pg, &r[i]);
    u = math::sve::erfinv(u, svptrue_b32());
    u = svmla_x(svptrue_b32(), svdup_f32(a), u, std::sqrt(2.0f) * sigma);
    svst1(pg, &r[i], u);
  }
}

template <> inline void icdf(int64_t n, double *r, double a, double sigma) {
  for (int64_t i = 0; i < n; i += svcntd()) {
    const auto pg = svwhilelt_b64(i, n);
    auto u = svld1(pg, &r[i]);
    u = math::sve::erfinv(u, svptrue_b64());
    u = svmla_x(svptrue_b64(), svdup_f64(a), u, std::sqrt(2.0) * sigma);
    svst1(pg, &r[i], u);
  }
}
} // namespace

template <> struct ICDF<CPUFeatures::sve> {
  template <openrng::FloatingPoint T>
  static int fill(Generator *generator, const int64_t n, T r[], const T a,
                  const T sigma) {
    assert(n > 0);
    assert(generator != nullptr);

    generator->fill(n, r, T{-1}, T{1});
    icdf(n, r, a, sigma);

    return VSL_ERROR_OK;
  }
};

template int ICDF<CPUFeatures::sve>::fill(Generator *, const int64_t, float[],
                                          const float, const float);
template int ICDF<CPUFeatures::sve>::fill(Generator *, const int64_t, double[],
                                          const double, const double);

} // namespace openrng::distribution::gaussian
