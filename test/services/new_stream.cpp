/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <climits>
#include <map>
#include <openrng.h>
#include <refng_vsl.h>
#include <supports_autogen.h>
#include <utils/names_map.hpp>

namespace {
template <int N>
using ExpectedDataMap = std::map<openrng_int_t, std::array<uint32_t, N>>;

ExpectedDataMap<2> expectedData = {
    {VSL_BRNG_MCG31,         {0x00000001, 0x43806c20}},
    {VSL_BRNG_MCG59,         {0x00000001, 0x00000000}},
    {VSL_BRNG_MRG32K3A,      {0x0022a011, 0x8e4e3264}},
    {VSL_BRNG_R250,          {0x3abafa6c, 0x97e10ec4}},
    {VSL_BRNG_MT19937,       {0xd82c07cd, 0x629f6fbe}},
    {VSL_BRNG_SFMT19937,     {0x5b889b2f, 0xda3af938}},
    {VSL_BRNG_PHILOX4X32X10, {0x6627e8d5, 0xe169c58d}},
    {VSL_BRNG_SOBOL,         {0x80000000, 0xc0000000}},
};
} // namespace

TEST_CASE("New stream invalid index") {
  VSLStreamStatePtr stream;
  const int errcode = vslNewStream(&stream, 0xff'ff'ff'ff, 0);
  REQUIRE(errcode == VSL_RNG_ERROR_INVALID_BRNG_INDEX);
}

TEST_CASE("New stream not implemented", "[openrng_only]") {
  const auto brngId = GENERATE(NEWSTREAM_NOTIMPLEMENTED);
  CAPTURE(brngToString(brngId));

  VSLStreamStatePtr stream;
  const int errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_FEATURE_NOT_IMPLEMENTED);
}

TEST_CASE("New stream fixed data") {
  VSLStreamStatePtr stream;
  VSLBRngProperties brngProperties;
  const int n = 2;
  int errcode;

  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED);
  CAPTURE(brngToString(brngId));

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int nWord = brngProperties.WordSize / sizeof(uint32_t);

  errcode = vslNewStream(&stream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::array<uint32_t, n> buffer;
  const openrng_int_t distMethod = VSL_RNG_METHOD_UNIFORMBITS_STD;
  viRngUniformBits(distMethod, stream, n / nWord, buffer.data());

  const auto expectedDatum = expectedData.at(brngId);
  REQUIRE(buffer == expectedDatum);

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("New stream reference test") {
  VSLStreamStatePtr vstream, rstream;
  VSLBRngProperties brngProperties;
  const auto valuesToGenerate = 600;
  /* 40 is not included arbitrarily - this ensures that every dimension is
     tested using the default params for SOBOL.  */
  const auto seed = GENERATE(as<openrng_int_t>{}, 0, 1, 2, 40, 0xffff, INT_MAX,
                             UINT_MAX, UINT_MAX + 1, UINT64_MAX);

  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED);
  const auto n = GENERATE_COPY(1, 2, 3, 4, 100, valuesToGenerate);
  int errcode;

  CAPTURE(brngToString(brngId), seed, n);

  errcode = vslGetBrngProperties(brngId, &brngProperties);
  REQUIRE(errcode == VSL_ERROR_OK);
  const int bufferSize = brngProperties.WordSize / sizeof(uint32_t) * n;

  errcode = vslNewStream(&vstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  // We test generation of valuesToGenerate numbers, either by filling a
  // buffer with valuesToGenerate numbers or less. To keep the testing simple,
  // we require valuesToGenerate to be divisible by n.
  REQUIRE((valuesToGenerate / n) * n == valuesToGenerate);

  for (int i = 0; i < valuesToGenerate; i += n) {
    CAPTURE(i);
    std::vector<uint32_t> vslbuffer(bufferSize);
    errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, vstream, n,
                               vslbuffer.data());
    REQUIRE(errcode == VSL_ERROR_OK);
    std::vector<uint32_t> refbuffer(bufferSize);
    errcode = riRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, rstream, n,
                               refbuffer.data());
    REQUIRE(errcode == VSL_ERROR_OK);

    REQUIRE(vslbuffer == refbuffer);
  }

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
