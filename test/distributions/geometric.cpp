/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <openrng.h>
#include <refng_vsl.h>
#include <utils/golden/geometric.hpp>
#include <utils/names_map.hpp>

#include <cmath>
#include <vector>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

TEST_CASE("Geometric bad arguments", "[geometric][icdf]") {
  const int n = 100;
  const double p = 0.5;
  int buffer[n];

  VSLStreamStatePtr stream;
  int errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_MCG59, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode = viRngGeometric(0xffff, stream, n, buffer, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream)") {
    errcode =
        viRngGeometric(VSL_RNG_METHOD_GEOMETRIC_ICDF, nullptr, n, buffer, p);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (size)") {
    errcode =
        viRngGeometric(VSL_RNG_METHOD_GEOMETRIC_ICDF, stream, -1, buffer, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (buffer)") {
    errcode =
        viRngGeometric(VSL_RNG_METHOD_GEOMETRIC_ICDF, stream, n, nullptr, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fifth parameter (p)") {
    const double bad_p = GENERATE_COPY(-1.1, 0., 1., 1.1);
    errcode =
        viRngGeometric(VSL_RNG_METHOD_GEOMETRIC_ICDF, stream, n, buffer, bad_p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

/**
 * The geometric distribution implemented by openrng is supported on the
 * set {0,1,2,...}. Hence, if any generated value was outside this range
 * (i.e. strictly less than 0), then the implementation would be erroneous.
 */
TEST_CASE("Geometric outputs only in the correct range", "[geometric][icdf]") {
  const int n = 100'000;
  const double p = GENERATE_COPY(0.001, 0.5, 0.999);
  CAPTURE(n, p);

  const auto brngId = VSL_BRNG_MCG59;
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<int> buffer(n);
  viRngGeometric(VSL_RNG_METHOD_GEOMETRIC_ICDF, stream, n, buffer.data(), p);

  for (int i = 0; i < n; i++) {
    REQUIRE(buffer[i] >= 0);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

/**
 * Direct comparison between the refng and vsl output sequences
 * for the same seed and p values. Due to floating-point errors and
 * casting to integer, we allow 0.001% of all elements generated in
 * vsl to be different by 1 from refng. The test passes if the fraction
 * is never greater than that.
 */
TEST_CASE("Geometric output is approximately comparable to refng",
          "[geometric][icdf]") {
  const int n = 1'000'000;
  const int allowedError = 1;
  const double allowedFailuresFraction = 1e-5;
  const double p = GENERATE_COPY(0.001, 0.5, 0.999);

  VSLStreamStatePtr vstream, rstream;
  const auto brngId = VSL_BRNG_MCG59;
  CAPTURE(brngToString(brngId));

  int errcode = vslNewStream(&vstream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<int> vslbuffer(n);
  errcode = viRngGeometric(VSL_RNG_METHOD_GEOMETRIC_ICDF, vstream, n,
                           vslbuffer.data(), p);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<int> refbuffer(n);
  errcode = riRngGeometric(VSL_RNG_METHOD_GEOMETRIC_ICDF, rstream, n,
                           refbuffer.data(), p);
  REQUIRE(errcode == VSL_ERROR_OK);

  int failures = 0;
  for (int i = 0; i < n; i++) {
    failures += std::abs(vslbuffer[i] - refbuffer[i]) <= 1 ? 0 : 1;
  }

  CAPTURE(failures, allowedFailuresFraction * n);
  REQUIRE(failures <= allowedFailuresFraction * n);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Geometric expected data", "[geometric][icdf][openrng_only]") {
  for (auto [key, reference] : GeometricGolden::reference) {
    const auto [brngId, method, seed, p] = key;
    const auto n = reference.size();
    CAPTURE(n, brngId, method, seed, p);

    VSLStreamStatePtr rstream;
    int errcode = refNewStream(&rstream, brngId, seed);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<int> buffer(n);
    errcode = riRngGeometric(method, rstream, n, buffer.data(), p);
    REQUIRE(errcode == VSL_ERROR_OK);

    REQUIRE(buffer == reference);

    errcode = refDeleteStream(&rstream);
    REQUIRE(errcode == VSL_ERROR_OK);
  }
}
