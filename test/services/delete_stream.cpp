/*
 * SPDX-FileCopyrightText: <text>Copyright 2024, 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_test_macros.hpp>
#include <openrng.h>
#include <vector>

TEST_CASE("Delete stream errors") {

  SECTION("Passing a nullptr as stream returns VSL_ERROR_NULL_PTR") {
    VSLStreamStatePtr stream = nullptr;
    const int errcode = vslDeleteStream(&stream);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Passing a buffer of nulls results in VSL_RNG_ERROR_BAD_STREAM") {
    // A large buffer of zeros, hopefully bigger than any Generator...
    std::vector buffer(1 << 20, 0);
    VSLStreamStatePtr stream = buffer.data();
    const int errcode = vslDeleteStream(&stream);
    REQUIRE(errcode == VSL_RNG_ERROR_BAD_STREAM);
  }
}

TEST_CASE("vslDeleteStream with nullptr returns VSL_ERROR_NULL_PTR",
          "[openrng_only]") {
  const int errcode = vslDeleteStream(nullptr);
  REQUIRE(errcode == VSL_ERROR_NULL_PTR);
}