/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once
#include <math.h>

/**
 * Inverse error function, double precision
 *
 * Routines for calculating the inverse error function, where the error function
 * is
 *
 *   erf(x) = (2 / \sqrt(\pi)) \int_{0}^{x} \exp(-t^2) dt
 *
 * Unfortunately no closed form exists for the inverse, but it can be
 * approximated by making the transformation
 *
 *   w = -\log(1 - x^2)
 *
 * The approach and constants are taken from
 *
 *   Michael B. Giles. 2011. Approximating the erfinv function.
 *
 * These routines are designed for a GPU. A SIMD friendly version is described
 * in
 *
 *   Michael B. Giles, Oliver Sheridan-Methven. 2023. Approximating inverse
 *   cumulative distribution functions to produce approximate random variables
 */
static inline double erfinv(double x) {
  double w, p;

  // Make the following transformation
  //
  //   w = -log(1 - x^2)
  //
  // The expression is expanded to avoid "catastrophic cancellation".
  //
  w = -log((1.0 - x) * (1.0 + x));

  if (w < 6.250000) {
    w = w - 3.125000;
    p = -3.6444120640178196996e-21;
    p = -1.685059138182016589e-19 + p * w;
    p = 1.2858480715256400167e-18 + p * w;
    p = 1.115787767802518096e-17 + p * w;
    p = -1.333171662854620906e-16 + p * w;
    p = 2.0972767875968561637e-17 + p * w;
    p = 6.6376381343583238325e-15 + p * w;
    p = -4.0545662729752068639e-14 + p * w;
    p = -8.1519341976054721522e-14 + p * w;
    p = 2.6335093153082322977e-12 + p * w;
    p = -1.2975133253453532498e-11 + p * w;
    p = -5.4154120542946279317e-11 + p * w;
    p = 1.051212273321532285e-09 + p * w;
    p = -4.1126339803469836976e-09 + p * w;
    p = -2.9070369957882005086e-08 + p * w;
    p = 4.2347877827932403518e-07 + p * w;
    p = -1.3654692000834678645e-06 + p * w;
    p = -1.3882523362786468719e-05 + p * w;
    p = 0.0001867342080340571352 + p * w;
    p = -0.00074070253416626697512 + p * w;
    p = -0.0060336708714301490533 + p * w;
    p = 0.24015818242558961693 + p * w;
    p = 1.6536545626831027356 + p * w;
  } else if (w < 16.000000) {
    w = sqrt(w) - 3.250000;
    p = 2.2137376921775787049e-09;
    p = 9.0756561938885390979e-08 + p * w;
    p = -2.7517406297064545428e-07 + p * w;
    p = 1.8239629214389227755e-08 + p * w;
    p = 1.5027403968909827627e-06 + p * w;
    p = -4.013867526981545969e-06 + p * w;
    p = 2.9234449089955446044e-06 + p * w;
    p = 1.2475304481671778723e-05 + p * w;
    p = -4.7318229009055733981e-05 + p * w;
    p = 6.8284851459573175448e-05 + p * w;
    p = 2.4031110387097893999e-05 + p * w;
    p = -0.0003550375203628474796 + p * w;
    p = 0.00095328937973738049703 + p * w;
    p = -0.0016882755560235047313 + p * w;
    p = 0.0024914420961078508066 + p * w;
    p = -0.0037512085075692412107 + p * w;
    p = 0.005370914553590063617 + p * w;
    p = 1.0052589676941592334 + p * w;
    p = 3.0838856104922207635 + p * w;
  } else {
    w = sqrt(w) - 5.000000;
    p = -2.7109920616438573243e-11;
    p = -2.5556418169965252055e-10 + p * w;
    p = 1.5076572693500548083e-09 + p * w;
    p = -3.7894654401267369937e-09 + p * w;
    p = 7.6157012080783393804e-09 + p * w;
    p = -1.4960026627149240478e-08 + p * w;
    p = 2.9147953450901080826e-08 + p * w;
    p = -6.7711997758452339498e-08 + p * w;
    p = 2.2900482228026654717e-07 + p * w;
    p = -9.9298272942317002539e-07 + p * w;
    p = 4.5260625972231537039e-06 + p * w;
    p = -1.9681778105531670567e-05 + p * w;
    p = 7.5995277030017761139e-05 + p * w;
    p = -0.00021503011930044477347 + p * w;
    p = -0.00013871931833623122026 + p * w;
    p = 1.0103004648645343977 + p * w;
    p = 4.8499064014085844221 + p * w;
  }

  return p * x;
}

/**
 * Inverse error function, single precision
 *
 * See erfinv(double x).
 */
static inline float erfinv(float x) {
  float w, p;

  // Make the following transformation
  //
  //   w = -log(1 - x^2)
  //
  // The difference is expanded to avoid "catastrophic cancellation".
  //
  w = -logf((1.0f - x) * (1.0f + x));
  if (w < 5.000000f) {
    w = w - 2.500000f;
    p = 2.81022636e-08f;
    p = 3.43273939e-07f + p * w;
    p = -3.5233877e-06f + p * w;
    p = -4.39150654e-06f + p * w;
    p = 0.00021858087f + p * w;
    p = -0.00125372503f + p * w;
    p = -0.00417768164f + p * w;
    p = 0.246640727f + p * w;
    p = 1.50140941f + p * w;
  } else {
    w = sqrtf(w) - 3.000000f;
    p = -0.000200214257f;
    p = 0.000100950558f + p * w;
    p = 0.00134934322f + p * w;
    p = -0.00367342844f + p * w;
    p = 0.00573950773f + p * w;
    p = -0.0076224613f + p * w;
    p = 0.00943887047f + p * w;
    p = 1.00167406f + p * w;
    p = 2.83297682f + p * w;
  }
  return p * x;
}
