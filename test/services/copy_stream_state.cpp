/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <openrng.h>
#include <supports_autogen.h>
#include <utils/detect_trng.hpp>
#include <utils/names_map.hpp>

TEST_CASE("Copy stream state basic") {
  VSLStreamStatePtr oldStream, newStream;
  VSLBRngProperties brngProperties;
  const int n = 1000;
  const int oldSeed = 0;
  const int newSeed = 0xff;
  int errcode;

  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED);
  CAPTURE(brngToString(brngId));

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int bufferSize = brngProperties.WordSize / sizeof(uint32_t) * n;

  errcode = vslNewStream(&oldStream, brngId, oldSeed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&newStream, brngId, newSeed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslCopyStreamState(newStream, oldStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> oldBuffer(bufferSize);
  std::vector<uint32_t> newBuffer(bufferSize);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, oldStream, n,
                             oldBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, newStream, n,
                             newBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(oldBuffer == newBuffer);

  errcode = vslDeleteStream(&oldStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&newStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Copy stream state skip ahead") {
  VSLStreamStatePtr oldStream, newStream;
  VSLBRngProperties brngProperties;
  const int n = 1000;
  const int nSkip = 1000;
  const int oldSeed = 0;
  const int newSeed = 0xff;
  int errcode;

  const auto brngId = GENERATE(SKIPAHEAD_SUPPORTED);
  CAPTURE(brngToString(brngId));

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int bufferSize = brngProperties.WordSize / sizeof(uint32_t) * n;

  errcode = vslNewStream(&oldStream, brngId, oldSeed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(oldStream, nSkip);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&newStream, brngId, newSeed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslCopyStreamState(newStream, oldStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> oldBuffer(bufferSize);
  std::vector<uint32_t> newBuffer(bufferSize);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, oldStream, n,
                             oldBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, newStream, n,
                             newBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(oldBuffer == newBuffer);

  errcode = vslDeleteStream(&oldStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&newStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Copy stream state leap frog") {
  VSLStreamStatePtr oldStream, newStream;
  VSLBRngProperties brngProperties;
  const int n = 1000;
  const int threadId = 11;
  const int threadCount = 64;
  const int oldSeed = 0;
  const int newSeed = 0xff;
  int errcode;

  const auto brngId = GENERATE(LEAPFROG_SUPPORTED);
  CAPTURE(brngToString(brngId));

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int bufferSize = brngProperties.WordSize / sizeof(uint32_t) * n;

  errcode = vslNewStream(&oldStream, brngId, oldSeed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&newStream, brngId, newSeed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslCopyStreamState(newStream, oldStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> oldBuffer(bufferSize);
  std::vector<uint32_t> newBuffer(bufferSize);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, oldStream, n,
                             oldBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, newStream, n,
                             newBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(oldBuffer == newBuffer);

  errcode = vslDeleteStream(&oldStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&newStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Copy stream state bad src stream") {
  VSLStreamStatePtr stream;
  const int nullBufferSize = 4096;
  int errcode;

  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED, NEWSTREAM_NONDETERM);
  CAPTURE(brngToString(brngId));

  errcode = vslNewStream(&stream, brngId, 0);
  ALLOW_NONDETERM_UNSUPPORTED_IF_NO_TRNG(brngId, errcode);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<char> nullBuffer(nullBufferSize, 0);

  SECTION("Bad source stream") {
    errcode = vslCopyStreamState(stream, nullBuffer.data());
    REQUIRE(errcode == VSL_RNG_ERROR_BAD_STREAM);
  }

  SECTION("Bad dest stream") {
    errcode = vslCopyStreamState(nullBuffer.data(), stream);
    REQUIRE(errcode == VSL_RNG_ERROR_BAD_STREAM);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Copy stream state nullptr") {
  VSLStreamStatePtr stream;
  int errcode;

  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED, NEWSTREAM_NONDETERM);
  CAPTURE(brngToString(brngId));

  errcode = vslNewStream(&stream, brngId, 0);
  ALLOW_NONDETERM_UNSUPPORTED_IF_NO_TRNG(brngId, errcode);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Bad source stream") {
    errcode = vslCopyStreamState(stream, nullptr);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Bad dest stream") {
    errcode = vslCopyStreamState(nullptr, stream);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Copy stream incompatible", "[openrng_only]") {
  VSLStreamStatePtr srcStream;
  VSLStreamStatePtr dstStream;
  int errcode;

  /* Skipping cases when TRNG is unsupported leads to memory leaks in the first
   * stream, so it is necessary in this case to dynamically select which
   * generators to test.  */
  decltype(GENERATE(NEWSTREAM_SUPPORTED)) srcMethod, dstMethod;
  if (trng_is_supported()) {
    srcMethod = GENERATE(NEWSTREAM_SUPPORTED, NEWSTREAM_NONDETERM);
    dstMethod = GENERATE(NEWSTREAM_SUPPORTED, NEWSTREAM_NONDETERM);
  } else {
    srcMethod = GENERATE(NEWSTREAM_SUPPORTED);
    dstMethod = GENERATE(NEWSTREAM_SUPPORTED);
  }

  CAPTURE(srcMethod, dstMethod);

  // Only run the test for differing brngId
  if (srcMethod != dstMethod) {
    errcode = vslNewStream(&srcStream, srcMethod, 0);
    REQUIRE(errcode == VSL_ERROR_OK);

    errcode = vslNewStream(&dstStream, dstMethod, 0);
    REQUIRE(errcode == VSL_ERROR_OK);

    errcode = vslCopyStreamState(dstStream, srcStream);
    REQUIRE(errcode == VSL_RNG_ERROR_BRNGS_INCOMPATIBLE);

    errcode = vslDeleteStream(&dstStream);
    REQUIRE(errcode == VSL_ERROR_OK);

    errcode = vslDeleteStream(&srcStream);
    REQUIRE(errcode == VSL_ERROR_OK);
  }
}
