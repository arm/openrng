/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
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

TEST_CASE("Uniform bits 32 not supported") {
  const auto brngId = GENERATE(UNIFORMBITS32_UNSUPPORTED);
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  unsigned int buffer[2];
  const auto distMethod = VSL_RNG_METHOD_UNIFORMBITS32_STD;
  errcode = viRngUniformBits32(distMethod, stream, 2, buffer);
  REQUIRE(errcode == VSL_RNG_ERROR_BRNG_NOT_SUPPORTED);

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

#ifdef UNIFORMBITS32_NOTIMPLEMENTED
TEST_CASE("Uniform bits 32 not implemented", "[openrng_only]") {
  const auto brngId = GENERATE(UNIFORMBITS32_NOTIMPLEMENTED);
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  unsigned int buffer[2];
  const auto distMethod = VSL_RNG_METHOD_UNIFORMBITS32_STD;
  errcode = viRngUniformBits32(distMethod, stream, 2, buffer);
  REQUIRE(errcode == VSL_ERROR_FEATURE_NOT_IMPLEMENTED);

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
#endif

TEST_CASE("Uniform bits 32 reference test") {
  VSLStreamStatePtr vstream, rstream;
  const auto seed = GENERATE(0, 1, 2, 0xffff, INT_MAX, UINT_MAX);
  const auto brngId = GENERATE(UNIFORMBITS32_SUPPORTED);
  const int n = GENERATE(10, 1000);

  CAPTURE(brngToString(brngId), seed, n);

  int errcode;
  errcode = vslNewStream(&vstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> vslbuffer(n);
  errcode = viRngUniformBits32(VSL_RNG_METHOD_UNIFORMBITS32_STD, vstream, n,
                               vslbuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> refbuffer(n);
  errcode = riRngUniformBits32(VSL_RNG_METHOD_UNIFORMBITS32_STD, rstream, n,
                               refbuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(vslbuffer == refbuffer);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
