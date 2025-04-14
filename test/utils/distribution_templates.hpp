/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <openrng.h>
#include <refng_vsl.h>

namespace {

template <typename T>
using rngCauchyFunction = int(const openrng_int_t method,
                              VSLStreamStatePtr stream, const openrng_int_t n,
                              T r[], const T a, const T beta);

template <typename T> rngCauchyFunction<T> *vRngCauchy;
template <> auto vRngCauchy<float> = vsRngCauchy;
template <> auto vRngCauchy<double> = vdRngCauchy;

template <typename T> rngCauchyFunction<T> *rRngCauchy;
template <> auto rRngCauchy<float> = rsRngCauchy;
template <> auto rRngCauchy<double> = rdRngCauchy;

template <typename T>
using rngExponentialFunction = int(const openrng_int_t method,
                                   VSLStreamStatePtr stream,
                                   const openrng_int_t n, T r[], const T a,
                                   const T beta);

template <typename T> rngExponentialFunction<T> *vRngExponential;
template <> auto vRngExponential<float> = vsRngExponential;
template <> auto vRngExponential<double> = vdRngExponential;

template <typename T> rngExponentialFunction<T> *rRngExponential;
template <> auto rRngExponential<float> = rsRngExponential;
template <> auto rRngExponential<double> = rdRngExponential;

template <typename T>
using rngGaussianFunction = int(const openrng_int_t method,
                                VSLStreamStatePtr stream, const openrng_int_t n,
                                T r[], const T a, const T sigma);

template <typename T> rngGaussianFunction<T> *vRngGaussian;
template <> const auto vRngGaussian<float> = vsRngGaussian;
template <> const auto vRngGaussian<double> = vdRngGaussian;

template <typename T> rngGaussianFunction<T> *rRngGaussian;
template <> const auto rRngGaussian<float> = rsRngGaussian;
template <> const auto rRngGaussian<double> = rdRngGaussian;

template <typename T>
using rngGaussianMVFunction = int(const openrng_int_t method,
                                  VSLStreamStatePtr stream,
                                  const openrng_int_t n, T r[],
                                  const openrng_int_t dimen,
                                  const openrng_int_t mstorage, const T *a,
                                  const T *t);

template <typename T> rngGaussianMVFunction<T> *vRngGaussianMV;
template <> const auto vRngGaussianMV<float> = vsRngGaussianMV;
template <> const auto vRngGaussianMV<double> = vdRngGaussianMV;

template <typename T> rngGaussianMVFunction<T> *rRngGaussianMV;
template <> const auto rRngGaussianMV<float> = rsRngGaussianMV;
template <> const auto rRngGaussianMV<double> = rdRngGaussianMV;

template <typename T>
using rngGumbelFunction = int(const openrng_int_t method,
                              VSLStreamStatePtr stream, const openrng_int_t n,
                              T r[], const T a, const T beta);

template <typename T> rngGumbelFunction<T> *vRngGumbel;
template <> auto vRngGumbel<float> = vsRngGumbel;
template <> auto vRngGumbel<double> = vdRngGumbel;

template <typename T> rngGumbelFunction<T> *rRngGumbel;
template <> auto rRngGumbel<float> = rsRngGumbel;
template <> auto rRngGumbel<double> = rdRngGumbel;

template <typename T>
using rngLaplaceFunction = int(const openrng_int_t method,
                               VSLStreamStatePtr stream, const openrng_int_t n,
                               T r[], const T a, const T beta);

template <typename T> rngLaplaceFunction<T> *vRngLaplace;
template <> auto vRngLaplace<float> = vsRngLaplace;
template <> auto vRngLaplace<double> = vdRngLaplace;

template <typename T> rngLaplaceFunction<T> *rRngLaplace;
template <> auto rRngLaplace<float> = rsRngLaplace;
template <> auto rRngLaplace<double> = rdRngLaplace;

template <typename T>
using rngRayleighFunction = int(const openrng_int_t method,
                                VSLStreamStatePtr stream, const openrng_int_t n,
                                T r[], const T a, const T beta);

template <typename T> rngRayleighFunction<T> *vRngRayleigh;
template <> auto vRngRayleigh<float> = vsRngRayleigh;
template <> auto vRngRayleigh<double> = vdRngRayleigh;

template <typename T> rngRayleighFunction<T> *rRngRayleigh;
template <> auto rRngRayleigh<float> = rsRngRayleigh;
template <> auto rRngRayleigh<double> = rdRngRayleigh;

template <typename T>
using rngUniformFunction = int(const openrng_int_t method,
                               VSLStreamStatePtr stream, const openrng_int_t n,
                               T r[], const T a, const T sigma);

template <typename T> rngUniformFunction<T> *vRngUniform;
template <> const auto vRngUniform<float> = vsRngUniform;
template <> const auto vRngUniform<double> = vdRngUniform;
template <> const auto vRngUniform<int> = viRngUniform;

template <typename T> rngUniformFunction<T> *rRngUniform;
template <> const auto rRngUniform<float> = rsRngUniform;
template <> const auto rRngUniform<double> = rdRngUniform;
template <> const auto rRngUniform<int> = riRngUniform;

template <typename T>
using rngWeibullFunction = int(const openrng_int_t method,
                               VSLStreamStatePtr stream, const openrng_int_t n,
                               T r[], const T alpha, const T a, const T beta);

template <typename T> rngWeibullFunction<T> *vRngWeibull;
template <> auto vRngWeibull<float> = vsRngWeibull;
template <> auto vRngWeibull<double> = vdRngWeibull;

template <typename T> rngWeibullFunction<T> *rRngWeibull;
template <> auto rRngWeibull<float> = rsRngWeibull;
template <> auto rRngWeibull<double> = rdRngWeibull;

} // namespace
