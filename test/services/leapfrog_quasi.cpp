/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <supports_autogen.h>
#include <utils/names_map.hpp>

TEST_CASE("QRNG leapfrog cannot be larger than vector") {
  const auto brngId = GENERATE(LEAPFROG_QUASI);
  CAPTURE(brngToString(brngId));

  auto dim = GENERATE(1, 2, 8, 16);

  VSLStreamStatePtr stream;
  int errcode;

  errcode = vslNewStream(&stream, brngId, dim);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslLeapfrogStream(stream, dim + 1, 0x7fffffff);
  REQUIRE(errcode == VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED);

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Leapfrogged QRNG matches single-dimension stream") {
  const auto brngId = GENERATE(LEAPFROG_QUASI);
  CAPTURE(brngToString(brngId));

  const int nElems = 1000;

  VSLStreamStatePtr leapfroggedStream, singleDimStream;
  int errcode;

  errcode = vslNewStream(&singleDimStream, brngId, 1);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&leapfroggedStream, brngId, 40);
  REQUIRE(errcode == VSL_ERROR_OK);
  errcode = vslLeapfrogStream(leapfroggedStream, 0, 0x7fffffff);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> leapfroggedBuffer(nElems), singleDimBuffer(nElems);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, singleDimStream,
                             nElems, singleDimBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, leapfroggedStream,
                             nElems, leapfroggedBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(leapfroggedBuffer == singleDimBuffer);

  errcode = vslDeleteStream(&singleDimStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&leapfroggedStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Leapfrogged QRNG matches dimension of multi-dimension stream") {
  const auto brngId = GENERATE(LEAPFROG_QUASI);
  CAPTURE(brngToString(brngId));

  const int nElems = 100;

  const int ndims = GENERATE(11, 13, 16);
  const int choose_dim = GENERATE(3, 7, 8, 9);

  int errcode;
  VSLStreamStatePtr leapfroggedStream, refStream;
  errcode = vslNewStream(&leapfroggedStream, brngId, ndims);
  REQUIRE(errcode == VSL_ERROR_OK);
  errcode = vslLeapfrogStream(leapfroggedStream, choose_dim, 0x7fffffff);
  REQUIRE(errcode == VSL_ERROR_OK);

  /* Note this isn't a stream from refng, but a stream containing correct
     versions of the dimensions we are choosing in leapfroggedStream.  */
  errcode = vslNewStream(&refStream, brngId, 40);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> leapfroggedBuffer(nElems), refBuffer(nElems * 40);
  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, leapfroggedStream,
                             nElems, leapfroggedBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, refStream,
                             nElems * 40, refBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> stridedRefBuffer(nElems);

  for (int i = 0; i < nElems; i++)
    stridedRefBuffer[i] = refBuffer[i * 40 + choose_dim];

  REQUIRE(stridedRefBuffer == leapfroggedBuffer);

  errcode = vslDeleteStream(&leapfroggedStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&refStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Leapfrogged QRNG starts from correct point after skipahead") {
  const auto brngId = GENERATE(LEAPFROG_QUASI);
  CAPTURE(brngToString(brngId));

  const int nElems = 100;

  const int ndims = GENERATE(8, 11);
  const int choose_dim = GENERATE(3, 4, 5);
  const int skip_ahead = GENERATE(0, 1, 4, 24, 35);

  int errcode;
  VSLStreamStatePtr leapfroggedStream, refStream;
  errcode = vslNewStream(&leapfroggedStream, brngId, ndims);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(leapfroggedStream, skip_ahead);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslLeapfrogStream(leapfroggedStream, choose_dim, 0x7fffffff);
  REQUIRE(errcode == VSL_ERROR_OK);

  /* Note this isn't a stream from refng, but a stream containing correct
     versions of the dimensions we are choosing in leapfroggedStream.  */
  errcode = vslNewStream(&refStream, brngId, 40);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> leapfroggedBuffer(nElems),
      refBuffer((nElems + skip_ahead) * 40);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, leapfroggedStream,
                             nElems, leapfroggedBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, refStream,
                             (nElems + skip_ahead) * 40, refBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> stridedRefBuffer(nElems);

  for (int i = 0; i < nElems; i++) {
    /* Sample should begin from the first vector for which element choose_dim
       has not been skipped past.  */
    int num_full_vecs_to_skip = i + skip_ahead / ndims;
    if (skip_ahead % ndims > choose_dim)
      num_full_vecs_to_skip += 1;
    stridedRefBuffer[i] = refBuffer[num_full_vecs_to_skip * 40 + choose_dim];
  }

  REQUIRE(leapfroggedBuffer == stridedRefBuffer);

  errcode = vslDeleteStream(&leapfroggedStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&refStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Leapfrogged QRNG starts from correct point after sample") {
  const auto brngId = GENERATE(LEAPFROG_QUASI);
  CAPTURE(brngToString(brngId));

  const int nElems = 100;

  const int ndims = GENERATE(8, 11);
  const int choose_dim = GENERATE(3, 4, 5);
  const int pre_sample_size = GENERATE(0, 1, 4, 22, 24, 35);

  int errcode;
  VSLStreamStatePtr leapfroggedStream, refStream;
  errcode = vslNewStream(&leapfroggedStream, brngId, ndims);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> wastedBuffer(pre_sample_size);
  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, leapfroggedStream,
                             pre_sample_size, wastedBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslLeapfrogStream(leapfroggedStream, choose_dim, 0x7fffffff);
  REQUIRE(errcode == VSL_ERROR_OK);

  /* Note this isn't a stream from refng, but a stream containing correct
     versions of the dimensions we are choosing in leapfroggedStream.  */
  errcode = vslNewStream(&refStream, brngId, 40);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> leapfroggedBuffer(nElems),
      refBuffer((nElems + pre_sample_size) * 40);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, leapfroggedStream,
                             nElems, leapfroggedBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, refStream,
                             (nElems + pre_sample_size) * 40, refBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> stridedRefBuffer(nElems);
  for (int i = 0; i < nElems; i++) {
    /* Sample should begin from the first vector for which element choose_dim
       has not been sampled past.  */
    int num_full_vecs_to_skip = i + pre_sample_size / ndims;
    if (pre_sample_size % ndims > choose_dim)
      num_full_vecs_to_skip += 1;
    stridedRefBuffer[i] = refBuffer[num_full_vecs_to_skip * 40 + choose_dim];
  }

  REQUIRE(stridedRefBuffer == leapfroggedBuffer);

  errcode = vslDeleteStream(&leapfroggedStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&refStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("QRNG leapfrogged skipahead - ragged skip not ignored",
          "[openrng_only]") {
  const auto brngId = GENERATE(LEAPFROG_QUASI);
  CAPTURE(brngToString(brngId));

  const int nElems = 100;

  const int ndims = GENERATE(8, 11);
  const int choose_dim = GENERATE(3, 4, 5);
  const int skip_ahead = GENERATE(0, 1, 3, 4, 8, 9, 25);

  int errcode;
  VSLStreamStatePtr leapfroggedStream, refStream;
  errcode = vslNewStream(&leapfroggedStream, brngId, ndims);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&refStream, brngId, 40);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslLeapfrogStream(leapfroggedStream, choose_dim, 0x7fffffff);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(leapfroggedStream, skip_ahead);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> leapfroggedBuffer(nElems),
      refBuffer((nElems + skip_ahead + choose_dim) * 40);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, leapfroggedStream,
                             nElems, leapfroggedBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, refStream,
                             (nElems + skip_ahead) * 40 + choose_dim,
                             refBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> stridedRefBuffer(nElems);

  for (int i = 0; i < nElems; i++) {
    /* Skipahead by n on a leapfrogged buffer means skipping n/ndims full
       vectors, where the division is rounded down. If the skipahead is ragged
       (not a multiple of the dimension), then advance one more full vector as
       long as the ragged part went past the chosen dimension.  */
    int num_to_skip = i + skip_ahead / ndims;
    if (skip_ahead % ndims > choose_dim)
      num_to_skip++;
    stridedRefBuffer[i] = refBuffer[num_to_skip * 40 + choose_dim];
  }

  REQUIRE(stridedRefBuffer == leapfroggedBuffer);

  errcode = vslDeleteStream(&leapfroggedStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&refStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("QRNG leapfrogged skipahead - ragged skip ignored", "[no_openrng]") {
  const auto brngId = GENERATE(LEAPFROG_QUASI);
  CAPTURE(brngToString(brngId));

  const int nElems = 100;

  const int ndims = GENERATE(8, 11);
  const int choose_dim = GENERATE(3, 4, 5);
  const int skip_ahead = GENERATE(0, 1, 3, 4, 8, 9, 25);

  int errcode;
  VSLStreamStatePtr leapfroggedStream, refStream;
  errcode = vslNewStream(&leapfroggedStream, brngId, ndims);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&refStream, brngId, 40);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslLeapfrogStream(leapfroggedStream, choose_dim, 0x7fffffff);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(leapfroggedStream, skip_ahead);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> leapfroggedBuffer(nElems),
      refBuffer((nElems + skip_ahead + choose_dim) * 40);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, leapfroggedStream,
                             nElems, leapfroggedBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, refStream,
                             (nElems + skip_ahead) * 40 + choose_dim,
                             refBuffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> stridedRefBuffer(nElems);

  for (int i = 0; i < nElems; i++) {
    /* Skipahead by n on a leapfrogged buffer means skipping n/ndims full
       vectors, where the division is rounded down, completely ignoring the
       ragged part.  */
    const int num_to_skip = i + skip_ahead / ndims;
    stridedRefBuffer[i] = refBuffer[num_to_skip * 40 + choose_dim];
  }

  REQUIRE(stridedRefBuffer == leapfroggedBuffer);

  errcode = vslDeleteStream(&leapfroggedStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&refStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("QRNG leapfrogged skipahead duplicate") {
  const auto brngId = GENERATE(LEAPFROG_QUASI);
  CAPTURE(brngToString(brngId));

  const int nElems = 100;

  const int ndims = GENERATE(8, 11);
  const int choose_dim = GENERATE(3, 4, 5);
  const int skip_ahead_1 = GENERATE(0, 1, 3, 25);
  const int skip_ahead_2 = GENERATE(31, 4, 9, 0, 12);

  int errcode;
  VSLStreamStatePtr one_skip_stream, two_skip_stream;
  errcode = vslNewStream(&one_skip_stream, brngId, ndims);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&two_skip_stream, brngId, ndims);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslLeapfrogStream(one_skip_stream, choose_dim, 0x7fffffff);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslLeapfrogStream(two_skip_stream, choose_dim, 0x7fffffff);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(one_skip_stream, skip_ahead_1 + skip_ahead_2);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(two_skip_stream, skip_ahead_1);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(two_skip_stream, skip_ahead_2);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> one_skip_buffer(nElems), two_skip_buffer(nElems);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, one_skip_stream,
                             nElems, one_skip_buffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, two_skip_stream,
                             nElems, two_skip_buffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(one_skip_buffer == two_skip_buffer);

  errcode = vslDeleteStream(&one_skip_stream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&two_skip_stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
