/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <refng_vsl.h>
#include <supports_autogen.h>
#include <utils/names_map.hpp>

TEST_CASE("Get the right brng properties") {
  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED, NEWSTREAM_NONDETERM);
  CAPTURE(brngToString(brngId));

  VSLBRngProperties vslProperties;
  vslGetBrngProperties(brngId, &vslProperties);

  VSLBRngProperties refProperties;
  refGetBrngProperties(brngId, &refProperties);

  REQUIRE(vslProperties.IncludesZero == refProperties.IncludesZero);
  REQUIRE(vslProperties.WordSize == refProperties.WordSize);
  REQUIRE(vslProperties.NBits == refProperties.NBits);
}

TEST_CASE("Check NSeeds property", "[openrng_only]") {
  /*
    The NSeeds property for a generator depends on its
    implementation and therefore the definition in OpenRNG
    may differ, for some generators, from other VSL libraries.
    However, it is important that the property is consistent
    across refng and vsl within OpenRNG.
  */
  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED, NEWSTREAM_NONDETERM);
  CAPTURE(brngToString(brngId));

  VSLBRngProperties vslProperties;
  vslGetBrngProperties(brngId, &vslProperties);

  VSLBRngProperties refProperties;
  vslGetBrngProperties(brngId, &refProperties);

  REQUIRE(vslProperties.NSeeds == refProperties.NSeeds);
}

TEST_CASE("Get the null brng properties", "[openrng_only]") {
  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED, NEWSTREAM_NONDETERM);
  CAPTURE(brngToString(brngId));

  VSLBRngProperties refProperties;
  vslGetBrngProperties(brngId, &refProperties);

  REQUIRE(refProperties.StreamStateSize == 0);
  REQUIRE(refProperties.InitStream == nullptr);
  REQUIRE(refProperties.sBRng == nullptr);
  REQUIRE(refProperties.dBRng == nullptr);
  REQUIRE(refProperties.iBRng == nullptr);
}

TEST_CASE("Stream not implemented", "[openrng_only]") {
  const auto brngId = GENERATE(NEWSTREAM_NOTIMPLEMENTED);
  CAPTURE(brngToString(brngId));

  VSLBRngProperties vslProperties;
  const int errcode = vslGetBrngProperties(brngId, &vslProperties);
  REQUIRE(errcode == VSL_ERROR_FEATURE_NOT_IMPLEMENTED);
}
