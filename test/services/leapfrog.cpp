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

TEST_CASE("Leap frog not supported") {
  const auto brngId = GENERATE(LEAPFROG_UNSUPPORTED);
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode;

  errcode = vslNewStream(&stream, brngId, 0);
  ALLOW_NONDETERM_UNSUPPORTED_IF_NO_TRNG(brngId, errcode);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslLeapfrogStream(stream, 0, 0);
  REQUIRE(errcode == VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED);

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

#ifdef LEAPFROG_NOTIMPLEMENTED
TEST_CASE("Leapfrog not implemented", "[openrng_only]") {
  const auto brngId = GENERATE(LEAPFROG_NOTIMPLEMENTED);
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr stream;
  int errcode;

  errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslLeapfrogStream(stream, 0, 0);
  REQUIRE(errcode == VSL_ERROR_FEATURE_NOT_IMPLEMENTED);

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
#endif

TEST_CASE("Leapfrog duplicate stream") {

  constexpr int n = 10;
  const int nSimulatedThreads = GENERATE(1, 3, 10);
  const int simulatedThreadId = GENERATE_COPY(range(0, nSimulatedThreads));
  const auto brngId = GENERATE(LEAPFROG_SUPPORTED);

  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr streamNoLeap;
  VSLStreamStatePtr streamLeap;
  VSLBRngProperties brngProperties;
  int errcode;

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int nWords = brngProperties.WordSize / sizeof(uint32_t);

  errcode = vslNewStream(&streamNoLeap, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&streamLeap, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Leapfrog splits the stream") {
    errcode =
        vslLeapfrogStream(streamLeap, simulatedThreadId, nSimulatedThreads);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<uint32_t> bufferNoLeap(nSimulatedThreads * nWords * n);
    errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, streamNoLeap,
                               nSimulatedThreads * n, bufferNoLeap.data());
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<uint32_t> bufferLeap(nWords * n);
    errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, streamLeap, n,
                               bufferLeap.data());
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<uint32_t> expectedBuffer;
    for (int i = simulatedThreadId * nWords; i < bufferNoLeap.size();
         i += nSimulatedThreads * nWords) {
      for (int j = 0; j < nWords; j++) {
        expectedBuffer.push_back(bufferNoLeap[i + j]);
      }
    }
    REQUIRE(expectedBuffer == bufferLeap);
  }

  SECTION("Subsequent leapfrog further splits the stream") {
    errcode =
        vslLeapfrogStream(streamLeap, simulatedThreadId, nSimulatedThreads);
    REQUIRE(errcode == VSL_ERROR_OK);

    errcode =
        vslLeapfrogStream(streamLeap, simulatedThreadId, nSimulatedThreads);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<uint32_t> bufferNoLeap(nSimulatedThreads * nSimulatedThreads *
                                       nWords * n);
    errcode =
        viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, streamNoLeap,
                         bufferNoLeap.size() / nWords, bufferNoLeap.data());
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<uint32_t> bufferLeap(nWords * n);
    errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, streamLeap, n,
                               bufferLeap.data());
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<uint32_t> expectedBuffer;
    for (int i = simulatedThreadId + simulatedThreadId * nSimulatedThreads;
         i < bufferNoLeap.size() / nWords;
         i += nSimulatedThreads * nSimulatedThreads) {
      for (int j = 0; j < nWords; j++) {
        expectedBuffer.push_back(bufferNoLeap[i * nWords + j]);
      }
    }
    REQUIRE(expectedBuffer == bufferLeap);
  }

  errcode = vslDeleteStream(&streamNoLeap);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&streamLeap);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Leapfrog reference test") {
  VSLStreamStatePtr vstream, rstream;
  VSLBRngProperties brngProperties;
  const auto seed = GENERATE(0, 1, 2, INT_MAX, UINT_MAX);
  const auto brngId = GENERATE(LEAPFROG_SUPPORTED);
  int errcode;

  CAPTURE(brngToString(brngId));

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int nWords = brngProperties.WordSize / sizeof(uint32_t);

  errcode = vslNewStream(&vstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Fills an array with the same 100 elements after leapfrog") {
    const int n = 100;
    const int nSimulatedThreads = GENERATE(1, 2, 3, 10);
    const int simulatedThreadId = GENERATE_COPY(range(0, nSimulatedThreads));
    std::vector<uint32_t> vslbuffer(n);
    std::vector<uint32_t> refbuffer(n);

    errcode = vslLeapfrogStream(vstream, simulatedThreadId, nSimulatedThreads);
    REQUIRE(errcode == VSL_ERROR_OK);

    errcode = refLeapfrogStream(rstream, simulatedThreadId, nSimulatedThreads);
    REQUIRE(errcode == VSL_ERROR_OK);

    errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, vstream,
                               n / nWords, vslbuffer.data());
    REQUIRE(errcode == VSL_ERROR_OK);

    errcode = riRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, rstream,
                               n / nWords, refbuffer.data());
    REQUIRE(errcode == VSL_ERROR_OK);

    REQUIRE(vslbuffer == refbuffer);

    SECTION("Output matches after a subsequent leapfrog") {
      const int n = 2;
      const int nSimulatedThreads2 = GENERATE(1, 2, 3, 10);
      const int simulatedThreadId2 =
          GENERATE_COPY(range(0, nSimulatedThreads2));

      errcode =
          vslLeapfrogStream(vstream, simulatedThreadId2, nSimulatedThreads2);
      REQUIRE(errcode == VSL_ERROR_OK);

      errcode =
          refLeapfrogStream(rstream, simulatedThreadId2, nSimulatedThreads2);
      REQUIRE(errcode == VSL_ERROR_OK);

      errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, vstream,
                                 n / nWords, vslbuffer.data());
      REQUIRE(errcode == VSL_ERROR_OK);

      errcode = riRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, rstream,
                                 n / nWords, refbuffer.data());
      REQUIRE(errcode == VSL_ERROR_OK);

      REQUIRE(vslbuffer == refbuffer);
    }

    SECTION("Output matches after a subsequent skipAhead") {
      const int n = 2;
      const int skipAhead = GENERATE(1, 2, 3, 10);

      errcode = vslSkipAheadStream(vstream, skipAhead);
      REQUIRE(errcode == VSL_ERROR_OK);

      errcode = refSkipAheadStream(rstream, skipAhead);
      REQUIRE(errcode == VSL_ERROR_OK);

      errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, vstream,
                                 n / nWords, vslbuffer.data());
      REQUIRE(errcode == VSL_ERROR_OK);

      errcode = riRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, rstream,
                                 n / nWords, refbuffer.data());
      REQUIRE(errcode == VSL_ERROR_OK);

      REQUIRE(vslbuffer == refbuffer);
    }
  }

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
