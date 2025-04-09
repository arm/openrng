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

TEST_CASE("Skip ahead not supported") {
  const auto brngId = GENERATE(SKIPAHEAD_UNSUPPORTED);
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode;

  errcode = vslNewStream(&stream, brngId, 0);
  ALLOW_NONDETERM_UNSUPPORTED_IF_NO_TRNG(brngId, errcode);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(stream, 0);
  REQUIRE(errcode == VSL_RNG_ERROR_SKIPAHEAD_UNSUPPORTED);

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

#ifdef SKIPAHEAD_NOTIMPLEMENTED
TEST_CASE("Skip ahead not implemented", "[openrng_only]") {
  const auto brngId = GENERATE(SKIPAHEAD_NOTIMPLEMENTED);
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode;

  errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(stream, 0);
  REQUIRE(errcode == VSL_ERROR_FEATURE_NOT_IMPLEMENTED);

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
#endif

TEST_CASE("Skip ahead duplicate stream") {
  VSLStreamStatePtr streamNoSkip;
  VSLStreamStatePtr streamSkip;
  VSLBRngProperties brngProperties;
  const auto brngId = GENERATE(SKIPAHEAD_SUPPORTED);
  const auto skipAhead = GENERATE(0LL, 1LL, 2LL, 31LL, 97LL, 1232LL, 91253LL);
  constexpr int n = 100;
  int errcode;

  CAPTURE(skipAhead, brngToString(brngId));

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int nWord = brngProperties.WordSize / sizeof(uint32_t);

  errcode = vslNewStream(&streamNoSkip, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&streamSkip, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(streamSkip, skipAhead);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> bufferNoSkip(nWord * (skipAhead + n));
  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, streamNoSkip,
                             skipAhead + n, bufferNoSkip.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> bufferSkip(nWord * n);
  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, streamSkip, n,
                             bufferSkip.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < bufferSkip.size(); i++) {
    CAPTURE(i);
    REQUIRE(bufferNoSkip[nWord * skipAhead + i] == bufferSkip[i]);
  }

  errcode = vslDeleteStream(&streamNoSkip);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&streamSkip);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Skip ahead reference test") {
  VSLStreamStatePtr vstream, rstream;
  VSLBRngProperties brngProperties;
  const auto seed = GENERATE(as<openrng_int_t>{}, 0, 1, 2, INT_MAX, UINT_MAX);
  const auto skipAhead =
      GENERATE(as<int64_t>{}, 0, 1, 2, 4, 23'456, 1LL << 32, LLONG_MAX);
  const int n = 100;
  int errcode;

  const auto brngId = GENERATE(SKIPAHEAD_SUPPORTED);
  CAPTURE(seed, skipAhead, brngToString(brngId));

  if (brngId == VSL_BRNG_SOBOL && skipAhead >= INT_MAX) {
    SKIP("Skipping past the end of SOBOL is not supported.");
  }

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int nWord = brngProperties.WordSize / sizeof(uint32_t);

  errcode = vslNewStream(&vstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(vstream, skipAhead);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refSkipAheadStream(rstream, skipAhead);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> vslbuffer(nWord * n);
  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, vstream, n,
                             vslbuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> refbuffer(nWord * n);
  errcode = riRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, rstream, n,
                             refbuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(vslbuffer == refbuffer);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
