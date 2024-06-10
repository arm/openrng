/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <openrng.h>
#include <refng_vsl.h>
#include <supports_autogen.h>
#include <utils/golden/bernoulli.hpp>
#include <utils/names_map.hpp>

#include <vector>

TEST_CASE("Bernoulli fills a buffer with expected data",
          "[bernoulli][openrng_only]") {
  for (auto [key, reference] : BernoulliGolden::reference) {
    const auto [brngId, method, seed, p] = key;
    const auto n = reference.size();
    CAPTURE(n, brngId, method, seed, p);

    VSLStreamStatePtr rstream;
    int errcode = refNewStream(&rstream, brngId, seed);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<int> buffer(n);
    errcode = riRngBernoulli(method, rstream, n, buffer.data(), p);
    REQUIRE(errcode == VSL_ERROR_OK);

    REQUIRE(buffer == reference);

    errcode = refDeleteStream(&rstream);
    REQUIRE(errcode == VSL_ERROR_OK);
  }
}

TEST_CASE("Bernoulli is bit comparable to refng", "[bernoulli]") {

  const double p = 0.5;

  VSLStreamStatePtr vstream, rstream;
  int errcode;

  // Assuming MCG31's float output is bit comparable between optimised and
  // unoptimised versions, which it is currently. The Bernoulli transformation
  // is so simple, that we can assert it will be bit comparable given the
  // generator is. This trick is useful because Bernoulli only outputs 1's and
  // 0's, making testing via tolerances difficult.
  const auto brngId = VSL_BRNG_MCG31;
  CAPTURE(brngToString(brngId));

  errcode = vslNewStream(&vstream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  const int n = 100'000;
  std::vector<int> vslbuffer(n);
  errcode = viRngBernoulli(VSL_RNG_METHOD_BERNOULLI_ICDF, vstream, n,
                           vslbuffer.data(), p);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<int> refbuffer(n);
  errcode = riRngBernoulli(VSL_RNG_METHOD_BERNOULLI_ICDF, rstream, n,
                           refbuffer.data(), p);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < vslbuffer.size(); i++) {
    CAPTURE(i);
    REQUIRE(vslbuffer[i] == refbuffer[i]);
  }

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Bernoulli bad arguments", "[bernoulli]") {
  const auto n = 100;
  const double p = 0.5;
  int buffer[n];

  VSLStreamStatePtr stream;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_R250, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode = viRngBernoulli(0xffff, stream, n, buffer, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream)") {
    errcode =
        viRngBernoulli(VSL_RNG_METHOD_BERNOULLI_ICDF, nullptr, n, buffer, p);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (size)") {
    errcode =
        viRngBernoulli(VSL_RNG_METHOD_BERNOULLI_ICDF, stream, -1, buffer, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (buffer)") {
    errcode =
        viRngBernoulli(VSL_RNG_METHOD_BERNOULLI_ICDF, stream, n, nullptr, p);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fifth parameter (p)") {
    // p < 0
    errcode =
        viRngBernoulli(VSL_RNG_METHOD_BERNOULLI_ICDF, stream, n, buffer, -0.1);
    REQUIRE(errcode == VSL_ERROR_BADARGS);

    // p > 1
    errcode =
        viRngBernoulli(VSL_RNG_METHOD_BERNOULLI_ICDF, stream, n, buffer, 1.1);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
