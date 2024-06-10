/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <supports_autogen.h>
#include <utils/detect_trng.hpp>

TEST_CASE("Get the right brngId") {
  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED, NEWSTREAM_NONDETERM);
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode;

  errcode = vslNewStream(&stream, brngId, 0);
  ALLOW_NONDETERM_UNSUPPORTED_IF_NO_TRNG(brngId, errcode);
  REQUIRE(errcode == VSL_ERROR_OK);

  const auto savedBrngId = vslGetStreamStateBrng(stream);

  REQUIRE(brngId == savedBrngId);

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
