/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once
#include <distributions/kernels/neon/binomial.hpp>
#include <distributions/kernels/neon/box_muller.hpp>
#include <distributions/kernels/neon/box_muller_2.hpp>
#include <distributions/kernels/neon/exponential.hpp>
#include <distributions/kernels/neon/gaussian_icdf.hpp>
#include <distributions/kernels/neon/gaussianmv.hpp>
#include <distributions/kernels/neon/geometric.hpp>
#include <distributions/kernels/neon/laplace.hpp>
#include <distributions/kernels/neon/poisson.hpp>
#include <distributions/kernels/neon/rayleigh.hpp>
#include <distributions/kernels/neon/weibull.hpp>

// We skip building SVE code on Windows & macOS. Toolchain support is limited.
#if !defined(_WIN32) && !defined(__APPLE__)
#include <distributions/kernels/sve/box_muller_2.hpp>
#include <distributions/kernels/sve/exponential.hpp>
#include <distributions/kernels/sve/gaussian_icdf.hpp>
#include <distributions/kernels/sve/geometric.hpp>
#include <distributions/kernels/sve/rayleigh.hpp>
#endif
