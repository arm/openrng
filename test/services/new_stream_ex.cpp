/*
 * SPDX-FileCopyrightText: <text>Copyright 2024, 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <climits>
#include <openrng.h>
#include <refng_vsl.h>
#include <supports_autogen.h>
#include <utils/names_map.hpp>

TEST_CASE("New stream ex invalid index") {
  VSLStreamStatePtr stream;
  uint32_t params[] = {0};
  const int errcode = vslNewStreamEx(&stream, 0xffff, 1, params);
  REQUIRE(errcode == VSL_RNG_ERROR_INVALID_BRNG_INDEX);
}

TEST_CASE("New stream ex reference test") {
  VSLStreamStatePtr vstream, rstream;
  VSLBRngProperties brngProperties;
  const auto params = GENERATE(
      std::vector<uint32_t>{0}, std::vector<uint32_t>{0, 0},
      std::vector<uint32_t>{0, 0, 0}, std::vector<uint32_t>{0, 0, 0, 0},
      std::vector<uint32_t>{0, 0, 0, 0, 0},
      std::vector<uint32_t>{0, 0, 0, 0, 0, 0},
      std::vector<uint32_t>{1, 1, 1, 0, 0, 0},
      std::vector<uint32_t>{0, 0, 0, 1, 1, 1},
      std::vector<uint32_t>{1, 2, 3, 4, 5, 6, 7},
      std::vector<uint32_t>(10000, 42));

  const int n = 1000;
  int errcode;

  const auto brngId = GENERATE(NEWSTREAMEX_SUPPORTED);
  CAPTURE(brngToString(brngId));

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int bufferSize = brngProperties.WordSize / sizeof(uint32_t) * n;

  errcode = vslNewStreamEx(&vstream, brngId, params.size(), params.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStreamEx(&rstream, brngId, params.size(), params.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> vslbuffer(bufferSize);
  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, vstream, n,
                             vslbuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> refbuffer(bufferSize);
  errcode = riRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, rstream, n,
                             refbuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(vslbuffer == refbuffer);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("New stream ex from nullptr stream state", "[openrng_only]") {
  uint32_t params[] = {0};
  int errcode = vslNewStreamEx(nullptr, 0xffff, 1, params);
  REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  errcode = refNewStreamEx(nullptr, 0xffff, 1, params);
  REQUIRE(errcode == VSL_ERROR_NULL_PTR);
}
