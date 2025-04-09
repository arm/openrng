/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <openrng.h>
#include <refng_vsl.h>

#include <array>
#include <vector>

TEST_CASE("MT2203 new stream reference test", "[openrng_only]") {
  const auto brngId = GENERATE(range(VSL_BRNG_MT2203, VSL_BRNG_MT2203 + 6024));
  const auto seed = GENERATE(0, 1, 0xfff);

  VSLStreamStatePtr vstream, rstream;
  int errcode;

  errcode = vslNewStream(&vstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  constexpr size_t nelems = 1000;
  std::array<uint32_t, nelems> vbuf, rbuf;

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, vstream, nelems,
                             vbuf.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = riRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, rstream, nelems,
                             rbuf.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(vbuf == rbuf);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("MT2203 new stream ex reference test", "[openrng_only]") {
  const auto brngId = GENERATE(range(VSL_BRNG_MT2203, VSL_BRNG_MT2203 + 6024));
  const auto params =
      GENERATE(std::vector<uint32_t>{0}, std::vector<uint32_t>{0, 0},
               std::vector<uint32_t>{1, 2, 3, 4, 5, 6, 7},
               std::vector<uint32_t>(10000, 42));

  VSLStreamStatePtr vstream, rstream;
  int errcode;

  errcode = vslNewStreamEx(&vstream, brngId, params.size(), params.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStreamEx(&rstream, brngId, params.size(), params.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  constexpr size_t nelems = 1000;
  std::array<uint32_t, nelems> vbuf, rbuf;

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, vstream, nelems,
                             vbuf.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = riRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, rstream, nelems,
                             rbuf.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(vbuf == rbuf);

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("MT2203 copy stream basic") {
  VSLStreamStatePtr oldStream, newStream;
  const int n = 100;
  int errcode;

  const auto brngId = GENERATE(range(VSL_BRNG_MT2203, VSL_BRNG_MT2203 + 6024));
  const auto seed = GENERATE(0, 1, 0xfff);

  errcode = vslNewStream(&oldStream, brngId, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslCopyStream(&newStream, oldStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> oldBuffer(n);
  std::vector<uint32_t> newBuffer(n);

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
