/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <openrng.h>
#include <refng_vsl.h>
#include <utils/detect_trng.hpp>
#include <utils/distribution_templates.hpp>

#include <algorithm>

#define SKIP_IF_UNSUPPORTED(err)                                               \
  ALLOW_NONDETERM_UNSUPPORTED_IF_NO_TRNG(VSL_BRNG_NONDETERM, err)

TEST_CASE("Nondeterm new stream") {
  VSLStreamStatePtr stream;
  int err = vslNewStream(&stream, VSL_BRNG_NONDETERM, 0);

  if (trng_is_supported()) {
    REQUIRE(err == VSL_ERROR_OK);
    err = vslDeleteStream(&stream);
    REQUIRE(err == VSL_ERROR_OK);
  } else {
    REQUIRE(err == VSL_RNG_ERROR_NONDETERM_NOT_SUPPORTED);
  }
}

TEST_CASE("Nondeterm subsequent takes") {
  /* Test that two subsequent takes do not yield the same values. It is
   * theoretically possible that this test could fail, however the likelihood is
   * so low we can consider it reliable.  */
  VSLStreamStatePtr stream;
  int err = vslNewStream(&stream, VSL_BRNG_NONDETERM, 0);
  SKIP_IF_UNSUPPORTED(err);

  /* Likelihood of failure = 0.5 ^ (32 * buffer_size)
     buffer_size == 10 -> likelihood of order 10^-97.  */
  const size_t buffer_size = 10;
  std::vector<uint32_t> buffer1(buffer_size), buffer2(buffer_size);

  err = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, stream, buffer_size,
                         buffer1.data());
  REQUIRE(err == VSL_ERROR_OK);

  err = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, stream, buffer_size,
                         buffer2.data());
  REQUIRE(err == VSL_ERROR_OK);

  REQUIRE(buffer1 != buffer2);

  err = vslDeleteStream(&stream);
  REQUIRE(err == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Nondeterm float", "[nondeterm]", float, double) {
  /* Test that all values are in the expected range. Also check that at least
   * one is above the mean and one below. Again, this test could fail, but n is
   * chosen such that the likelihood of this is negligible.  */

  VSLStreamStatePtr stream;
  int err = vslNewStream(&stream, VSL_BRNG_NONDETERM, 0);
  SKIP_IF_UNSUPPORTED(err);

  /* Likelihood of failure = 0.5 ^ buffer_size
     n = 250 -> likelihood failure of order 10^-76.  */
  const int n = 250;
  std::vector<TestType> buffer(n);
  err = vRngUniform<TestType>(VSL_RNG_METHOD_UNIFORM_STD, stream, n,
                              buffer.data(), 3, 5);
  REQUIRE(err == VSL_ERROR_OK);

  REQUIRE(std::all_of(buffer.begin(), buffer.end(),
                      [](TestType x) { return x >= 3 && x <= 5; }));

  REQUIRE(std::any_of(buffer.begin(), buffer.end(),
                      [](TestType x) { return x > 4; }));

  REQUIRE(std::any_of(buffer.begin(), buffer.end(),
                      [](TestType x) { return x < 4; }));

  err = vslDeleteStream(&stream);
  REQUIRE(err == VSL_ERROR_OK);
}

TEST_CASE("Nondeterm Copy stream basic") {
  /* No test that copies of streams match (because they won't) - just test that
   * copying works without error.  */
  VSLStreamStatePtr oldStream, newStream;
  int errcode;

  errcode = vslNewStream(&oldStream, VSL_BRNG_NONDETERM, 0);
  SKIP_IF_UNSUPPORTED(errcode);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslCopyStream(&newStream, oldStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&oldStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&newStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
