/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <openrng.h>
#include <refng_vsl.h>
#include <supports_autogen.h>
#include <utils/distribution_templates.hpp>
#include <utils/golden/uniform.hpp>
#include <utils/names_map.hpp>

#include <array>
#include <map>
#include <vector>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;
using Catch::Matchers::WithinULP;

namespace {
template <typename T>
void validateGolden(const std::vector<T> &buffer,
                    const std::vector<T> &reference);

template <>
void validateGolden<float>(const std::vector<float> &buffer,
                           const std::vector<float> &reference) {
  const int acceptedULP = 16;
  for (int i = 0; i < buffer.size(); i++) {
    REQUIRE_THAT(reference[i], WithinULP(buffer[i], acceptedULP));
  }
}

template <>
void validateGolden<double>(const std::vector<double> &buffer,
                            const std::vector<double> &reference) {
  const int acceptedULP = 256;
  for (int i = 0; i < buffer.size(); i++) {
    REQUIRE_THAT(reference[i], WithinULP(buffer[i], acceptedULP));
  }
}

template <>
void validateGolden<int>(const std::vector<int> &buffer,
                         const std::vector<int> &reference) {
  REQUIRE(buffer == reference);
}

template <typename T> std::pair<T, T> getBoundsForType();

template <> std::pair<float, float> getBoundsForType() {
  return {-2147483647.0, -1.0};
}

template <> std::pair<double, double> getBoundsForType() {
  return {-(1LL << 61), -1.0};
}

} // namespace

TEMPLATE_TEST_CASE("Uniform expected data", "[uniform]", float, double, int) {
  for (auto [key, reference] : UniformGolden::reference<TestType>()) {
    const int n = reference.size();
    const TestType a = key.a;
    const TestType b = key.b;
    const int seed = key.seed;
    const auto brngId = key.brngId;
    const auto method = key.method;
    CAPTURE(n, brngId, method, seed, a, b);

    VSLStreamStatePtr rstream;
    int errcode = refNewStream(&rstream, brngId, seed);
    REQUIRE(errcode == VSL_ERROR_OK);

    std::vector<TestType> buffer(n);
    errcode = rRngUniform<TestType>(method, rstream, n, buffer.data(), a, b);
    REQUIRE(errcode == VSL_ERROR_OK);

    validateGolden<TestType>(buffer, reference);

    errcode = refDeleteStream(&rstream);

    REQUIRE(errcode == VSL_ERROR_OK);
  }
}

TEMPLATE_TEST_CASE("Uniform is approximately comparable to refng", "[uniform]",
                   float, double, int) {

  const int lower = 1000;
  const int upper = 10000;

  VSLStreamStatePtr vstream, rstream;
  int errcode;

  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED);
  CAPTURE(brngToString(brngId));

  errcode = vslNewStream(&vstream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  const int n = 100;
  std::vector<TestType> vslbuffer(n);
  errcode = vRngUniform<TestType>(VSL_RNG_METHOD_UNIFORM_STD, vstream, n,
                                  vslbuffer.data(), lower, upper);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<TestType> refbuffer(n);
  errcode = rRngUniform<TestType>(VSL_RNG_METHOD_UNIFORM_STD, rstream, n,
                                  refbuffer.data(), lower, upper);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < vslbuffer.size(); i++) {
    CAPTURE(i);
    REQUIRE_THAT(vslbuffer[i], WithinAbs(refbuffer[i], 1));
  }

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Accurate uniform is approximately comparable to refng",
                   "[uniform]", float, double) {

  const int lower = 1000;
  const int upper = 10000;

  VSLStreamStatePtr vstream, rstream;
  int errcode;

  const auto brngId = GENERATE(NEWSTREAM_SUPPORTED);
  CAPTURE(brngToString(brngId));

  errcode = vslNewStream(&vstream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rstream, brngId, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  const int n = 100;
  std::vector<TestType> vslbuffer(n);
  errcode = vRngUniform<TestType>(VSL_RNG_METHOD_UNIFORM_STD_ACCURATE, vstream,
                                  n, vslbuffer.data(), lower, upper);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<TestType> refbuffer(n);
  errcode = rRngUniform<TestType>(VSL_RNG_METHOD_UNIFORM_STD_ACCURATE, rstream,
                                  n, refbuffer.data(), lower, upper);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < vslbuffer.size(); i++) {
    CAPTURE(i);
    REQUIRE_THAT(vslbuffer[i], WithinAbs(refbuffer[i], 1));
  }

  errcode = vslDeleteStream(&vstream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rstream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Uniform bad arguments", "[uniform]", float, double, int) {
  const auto n = 100;
  const TestType a = 0;
  const TestType b = 1;
  TestType buffer[n];

  VSLStreamStatePtr stream;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_R250, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode = vRngUniform<TestType>(0xffff, stream, n, buffer, a, b);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream)") {
    errcode = vRngUniform<TestType>(VSL_RNG_METHOD_UNIFORM_STD, nullptr, n,
                                    buffer, a, b);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (size)") {
    errcode = vRngUniform<TestType>(VSL_RNG_METHOD_UNIFORM_STD, stream, -1,
                                    buffer, a, b);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (buffer)") {
    errcode = vRngUniform<TestType>(VSL_RNG_METHOD_UNIFORM_STD, stream, n,
                                    nullptr, a, b);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fifth & sixth parameters (a & b)") {
    // a == b is invalid
    errcode = vRngUniform<TestType>(VSL_RNG_METHOD_UNIFORM_STD, stream, n,
                                    buffer, 2, 2);
    REQUIRE(errcode == VSL_ERROR_BADARGS);

    // a > b is invalid
    errcode = vRngUniform<TestType>(VSL_RNG_METHOD_UNIFORM_STD, stream, n,
                                    buffer, 2, 1);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("Accurate uniform for int returns bad arguments", "[uniform]") {
  const int n = 1;
  const int a = 0;
  const int b = 1;
  std::vector<int> buffer(n);

  VSLStreamStatePtr stream;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_R250, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = viRngUniform(VSL_RNG_METHOD_UNIFORM_STD_ACCURATE, stream, n,
                         buffer.data(), a, b);
  REQUIRE(errcode == VSL_ERROR_BADARGS);

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Accurate uniform is clamped to domain", "[uniform]", float,
                   double) {

  //
  // Get an upper and lower such that
  //
  //  upper - lower == -lower
  //
  // If the generator returns u == 1.0, then the uniform transformation
  //
  //   (upper - lower) * u + lower
  //
  // should return 0.0, which is outside the domain.
  //
  const auto [lower, upper] = getBoundsForType<TestType>();
  REQUIRE((upper - lower) * 1.0 == -lower);

  // We initialize MCG59 with (2^59 -1) so that the first value generated is
  // 1.0, the most extreme value. We still do an extensive search of the space.
  const unsigned int params[2] = {0xFFFFFFFF, 0x07FFFFFF};
  const auto brngId = VSL_BRNG_MCG59;
  const int N = 1 << 20;

  VSLStreamStatePtr stream;
  int errcode = vslNewStreamEx(&stream, brngId, 2, params);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<TestType> buffer(N);
  errcode = vRngUniform<TestType>(VSL_RNG_METHOD_UNIFORM_STD_ACCURATE, stream,
                                  buffer.size(), buffer.data(), lower, upper);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < N; i++) {
    CAPTURE(i);
    REQUIRE(buffer[i] <= upper);
    REQUIRE(buffer[i] >= lower);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
