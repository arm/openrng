/*
 * SPDX-FileCopyrightText: <text>Copyright 2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <openrng.h>
#include <refng_vsl.h>

TEST_CASE("Check library version", "[openrng_only]") {
  int o_major;
  int o_minor;
  int o_patch;
  openRngGetVersion(&o_major, &o_minor, &o_patch);

  REQUIRE(o_major == OPENRNG_MAJOR);
  REQUIRE(o_minor == OPENRNG_MINOR);
  REQUIRE(o_patch == OPENRNG_PATCH);
}