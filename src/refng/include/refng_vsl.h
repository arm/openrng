/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <openrng.h>
#include <stdint.h>

#if defined(OPENRNG_ILP64)
typedef long long refng_int_t;
typedef unsigned long long refng_uint_t;
#else
typedef int32_t refng_int_t;
typedef uint32_t refng_uint_t;
#endif
typedef unsigned long long refng_uint64_t;

#ifdef __cplusplus
extern "C" {
#endif

int rdRngBeta(const refng_int_t, VSLStreamStatePtr, const refng_int_t, double[],
              const double, const double, const double, const double);

int rdRngCauchy(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                double[], const double, const double);

int rdRngChiSquare(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                   double[], const int);

int rdRngExponential(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                     double[], const double, const double);

int rdRngGamma(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
               double[], const double, const double, const double);

int rdRngGaussian(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                  double[], const double, const double);

int rdRngGaussianMV(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                    double[], const refng_int_t, const refng_int_t,
                    const double *, const double *);

int rdRngGumbel(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                double[], const double, const double);

int rdRngLaplace(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                 double[], const double, const double);

int rdRngLognormal(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                   double[], const double, const double, const double,
                   const double);

int rdRngRayleigh(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                  double[], const double, const double);

int rdRngUniform(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                 double[], const double, const double);

int rdRngWeibull(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                 double[], const double, const double, const double);

int riRngBernoulli(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                   int[], const double);

int riRngBinomial(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                  int[], const int, const double);

int riRngGeometric(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                   int[], const double);

int riRngHypergeometric(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                        int[], const int, const int, const int);

int riRngMultinomial(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                     int[], const int, const int, const double[]);

int riRngNegBinomial(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                     int[], const double, const double);

int riRngNegbinomial(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                     int[], const double, const double);

int riRngPoisson(const refng_int_t, VSLStreamStatePtr, const refng_int_t, int[],
                 const double);

int riRngPoissonV(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                  int[], const double[]);

int riRngUniform(const refng_int_t, VSLStreamStatePtr, const refng_int_t, int[],
                 const int, const int);

int riRngUniformBits(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                     unsigned int[]);

int riRngUniformBits32(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                       unsigned int[]);

int riRngUniformBits64(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                       refng_uint64_t[]);

int rsRngBeta(const refng_int_t, VSLStreamStatePtr, const refng_int_t, float[],
              const float, const float, const float, const float);

int rsRngCauchy(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                float[], const float, const float);

int rsRngChiSquare(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                   float[], const int);

int rsRngExponential(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                     float[], const float, const float);

int rsRngGamma(const refng_int_t, VSLStreamStatePtr, const refng_int_t, float[],
               const float, const float, const float);

int rsRngGaussian(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                  float[], const float, const float);

int rsRngGaussianMV(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                    float[], const refng_int_t, const refng_int_t,
                    const float *, const float *);

int rsRngGumbel(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                float[], const float, const float);

int rsRngLaplace(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                 float[], const float, const float);

int rsRngLognormal(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                   float[], const float, const float, const float, const float);

int rsRngRayleigh(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                  float[], const float, const float);

int rsRngUniform(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                 float[], const float, const float);

int rsRngWeibull(const refng_int_t, VSLStreamStatePtr, const refng_int_t,
                 float[], const float, const float, const float);

int refCopyStream(VSLStreamStatePtr *, const VSLStreamStatePtr);

int refCopyStreamState(VSLStreamStatePtr, const VSLStreamStatePtr);

int refDeleteStream(VSLStreamStatePtr *);

int refGetBrngProperties(const int, VSLBRngProperties *);

int refGetNumRegBrngs(void);

int refGetStreamSize(const VSLStreamStatePtr);

int refGetStreamStateBrng(const VSLStreamStatePtr);

int refLeapfrogStream(VSLStreamStatePtr, const refng_int_t, const refng_int_t);

int refLoadStreamF(VSLStreamStatePtr *, const char *);

int refLoadStreamM(VSLStreamStatePtr *, const char *);

int refNewStream(VSLStreamStatePtr *, const refng_int_t, const refng_uint_t);

int refNewStreamEx(VSLStreamStatePtr *, const refng_int_t, const refng_int_t,
                   const unsigned int[]);

int refRegisterBrng(const VSLBRngProperties *);

int refSaveStreamF(const VSLStreamStatePtr, const char *);

int refSaveStreamM(const VSLStreamStatePtr, char *);

int refSkipAheadStream(VSLStreamStatePtr, const long long int);

int refSkipAheadStreamEx(VSLStreamStatePtr, const refng_int_t,
                         const refng_uint64_t[]);

int refdNewAbstractStream(VSLStreamStatePtr *, const refng_int_t,
                          const double[], const double, const double,
                          const dUpdateFuncPtr);

int refiNewAbstractStream(VSLStreamStatePtr *, const refng_int_t,
                          const unsigned int[], const iUpdateFuncPtr);

int refsNewAbstractStream(VSLStreamStatePtr *, const refng_int_t, const float[],
                          const float, const float, const sUpdateFuncPtr);

#ifdef __cplusplus
}
#endif
