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
#include <utils/detect_trng.hpp>
#include <utils/names_map.hpp>

TEST_CASE("Skip ahead ex not supported") {
  const auto brngId = GENERATE(SKIPAHEADEX_UNSUPPORTED);
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode;

  errcode = vslNewStream(&stream, brngId, 0);
  ALLOW_NONDETERM_UNSUPPORTED_IF_NO_TRNG(brngId, errcode);
  REQUIRE(errcode == VSL_ERROR_OK);

  openrng_uint64_t nskip[2] = {1, 1};
  errcode = vslSkipAheadStreamEx(stream, 2, nskip);
  REQUIRE(errcode == VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED);

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

#ifdef SKIPAHEADEX_NOTIMPLEMENTED
TEST_CASE("Skip ahead ex not implemented", "[openrng_only]") {
  const auto brngId = GENERATE(SKIPAHEADEX_NOTIMPLEMENTED);
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode;

  errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  openrng_uint64_t nskip[2] = {1, 1};
  errcode = vslSkipAheadStreamEx(stream, 2, nskip);
  REQUIRE(errcode == VSL_ERROR_FEATURE_NOT_IMPLEMENTED);

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
#endif

TEST_CASE("Skip ahead ex duplicate stream") {
  constexpr int buffer_size = 100;

  // Skip 2 * 2^192 - 1 elements
  const std::vector nskipOne{ULLONG_MAX, ULLONG_MAX, ULLONG_MAX, 1ULL};
  // Skip 2 * 2^192 elements
  const std::vector nskipTwo{0ULL, 0ULL, 0ULL, 2ULL};

  VSLStreamStatePtr streamOne;
  VSLStreamStatePtr streamTwo;
  int errcode;

  const auto brngId = GENERATE(SKIPAHEADEX_SUPPORTED);
  CAPTURE(brngToString(brngId));

  errcode = vslNewStream(&streamOne, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&streamTwo, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStreamEx(streamOne, nskipOne.size(), nskipOne.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStreamEx(streamTwo, nskipTwo.size(), nskipTwo.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> bufferOne(buffer_size + 1);
  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, streamOne,
                             buffer_size + 1, bufferOne.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> bufferTwo(buffer_size);
  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, streamTwo,
                             buffer_size, bufferTwo.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < buffer_size; i++) {
    // Second buffer should one element ahead of the first buffer
    REQUIRE(bufferOne[i + 1] == bufferTwo[i]);
  }

  errcode = vslDeleteStream(&streamOne);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&streamTwo);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Skip ahead ex reference test") {
  VSLStreamStatePtr vstream, rstream;
  const auto seed = GENERATE(0, 1, 2, INT_MAX, UINT_MAX);
  const auto brngId = GENERATE(SKIPAHEADEX_SUPPORTED);
  int errcode;

  CAPTURE(brngToString(brngId));

  errcode = vslNewStream(&vstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  const int n = 100;
  const auto nskip = GENERATE(std::vector{1ULL}, std::vector{0ULL, 1ULL},
                              std::vector{1ULL, 1ULL, 1ULL},
                              std::vector{92ULL, 82ULL, 732ULL, 21ULL, 21ULL},
                              std::vector{ULLONG_MAX, ULLONG_MAX, ULLONG_MAX,
                                          ULLONG_MAX, ULLONG_MAX, ULLONG_MAX},
                              std::vector{0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 42ULL,
                                          0ULL, 0ULL, 0ULL, 0ULL, 0ULL});

  std::vector<uint32_t> vslbuffer(n);

  errcode = vslSkipAheadStreamEx(vstream, nskip.size(), nskip.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refSkipAheadStreamEx(rstream, nskip.size(), nskip.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, vstream, n,
                             vslbuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> refbuffer(n);
  errcode = riRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, rstream, n,
                             refbuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(vslbuffer == refbuffer);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
