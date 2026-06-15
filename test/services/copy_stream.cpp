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

TEST_CASE("Copy stream basic") {
  VSLStreamStatePtr oldStream, newStream;
  VSLBRngProperties brngProperties;
  const int n = 1000;
  int errcode;

  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED);
  CAPTURE(brngToString(brngId));

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int bufferSize = brngProperties.WordSize / sizeof(uint32_t) * n;

  errcode = vslNewStream(&oldStream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslCopyStream(&newStream, oldStream);
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

TEST_CASE("Copy stream skip ahead") {
  VSLStreamStatePtr oldStream, newStream;
  VSLBRngProperties brngProperties;
  const int n = 1000;
  const int nSkip = 1000;
  const int seed = 0;
  int errcode;

  const auto brngId = GENERATE(SKIPAHEAD_SUPPORTED);
  CAPTURE(brngToString(brngId));

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int bufferSize = brngProperties.WordSize / sizeof(uint32_t) * n;

  errcode = vslNewStream(&oldStream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(oldStream, nSkip);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslCopyStream(&newStream, oldStream);
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

TEST_CASE("Copy stream leap frog") {
  VSLStreamStatePtr oldStream, newStream;
  VSLBRngProperties brngProperties;
  const int n = 1000;
  const int threadId = 11;
  const int threadCount = 64;
  const int seed = 0;
  int errcode;

  const auto brngId = GENERATE(LEAPFROG_SUPPORTED);
  CAPTURE(brngToString(brngId));

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int bufferSize = brngProperties.WordSize / sizeof(uint32_t) * n;

  errcode = vslNewStream(&oldStream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslCopyStream(&newStream, oldStream);
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

TEST_CASE("Copy stream bad stream") {
  VSLStreamStatePtr newStream;
  const int nullBufferSize = 4096;
  int errcode;

  std::vector<char> nullBuffer(nullBufferSize, 0);
  errcode = vslCopyStream(&newStream, nullBuffer.data());
  REQUIRE(errcode == VSL_RNG_ERROR_BAD_STREAM);
}

TEST_CASE("Copy stream from nullptr") {
  VSLStreamStatePtr newStream = nullptr;
  int errcode;

  errcode = vslCopyStream(&newStream, nullptr);
  REQUIRE(errcode == VSL_ERROR_NULL_PTR);

  errcode = refCopyStream(&newStream, nullptr);
  REQUIRE(errcode == VSL_ERROR_NULL_PTR);
}

TEST_CASE("Copy stream into nullptr", "[openrng_only]") {
  VSLStreamStatePtr oldStream = nullptr;
  int errcode;

  errcode = vslCopyStream(nullptr, oldStream);
  REQUIRE(errcode == VSL_ERROR_NULL_PTR);

  errcode = refCopyStream(nullptr, oldStream);
  REQUIRE(errcode == VSL_ERROR_NULL_PTR);
}