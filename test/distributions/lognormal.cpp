/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <array>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <map>
#include <openrng.h>
#include <refng_vsl.h>
#include <vector>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;
namespace {

template <typename T> const T normalisedError;
template <> const auto normalisedError<double> = pow(10, -7);
template <> const auto normalisedError<float> = powf(10, -4);

template <typename T>
using rngLognormalFunction = int(const openrng_int_t method,
                                 VSLStreamStatePtr stream,
                                 const openrng_int_t n, T r[], const T a,
                                 const T sigma, const T b, const T beta);

template <typename T>
using rngGaussianFunction = int(const openrng_int_t method,
                                VSLStreamStatePtr stream, const openrng_int_t n,
                                T r[], const T a, const T sigma);

template <typename T> rngLognormalFunction<T> *vRngLognormal;
template <> const auto vRngLognormal<float> = vsRngLognormal;
template <> const auto vRngLognormal<double> = vdRngLognormal;

template <typename T> rngGaussianFunction<T> *vRngGaussian;
template <> const auto vRngGaussian<float> = vsRngGaussian;
template <> const auto vRngGaussian<double> = vdRngGaussian;

template <typename T> rngLognormalFunction<T> *rRngLognormal;
template <> const auto rRngLognormal<float> = rsRngLognormal;
template <> const auto rRngLognormal<double> = rdRngLognormal;

template <typename T, int N>
using ExpectedDataMap = std::map<openrng_int_t, std::array<T, N>>;
template <typename T> ExpectedDataMap<T, 4> expectedData;

template <>
ExpectedDataMap<float, 4> expectedData<float> = {
    {VSL_RNG_METHOD_LOGNORMAL_BOXMULLER2,
     {0.325866, 0.00156646, 2.81869, 2.49477}                                      },
    {VSL_RNG_METHOD_LOGNORMAL_ICDF,       {0.00442722, 1.07104, 0.746267, 0.331774}}
};

template <>
ExpectedDataMap<double, 4> expectedData<double> = {
    {VSL_RNG_METHOD_LOGNORMAL_BOXMULLER2,
     {0.325865, 0.00156646, 2.81868, 2.49477}                                      },
    {VSL_RNG_METHOD_LOGNORMAL_ICDF,       {0.00219679, 1.07104, 0.746267, 0.331774}}
};

/**
 * Check whether a value is close enough to the target value.
 */
template <typename T> void check(T value, T target) {
  /*
    The calculation of a lognormal distribution involves exponentiating
    the value of a Gaussian distribution. This can cause large errors
    as small discrepancies in the output from the underlying Gaussian
    distribution become large discrepancies when being exponentiate.
    Also, we have to take into account the variable accuracies with
    which functions like `exp`, `sin`, `cos` are implemented for the
    different transformations and generation algorithm, see e.g.
    BOXMULLER2.

    For this reason, we accept discrepancies that are:

     1 in absolute value less than a `tol`, which can enfore correctness
       for small values of lognormal as the decimal values are more
       important;
     2 in relative value less than a `tol`, which can be suited for big
       values of lognormal distribution, where a 6th, or 7th, decimal
       point has not the same importance for large numbers.

    Either of these conditions has to be met for a comparison to be
    considered successful.

    In the tests below, as a Gaussian distribution is defined over the
    entire set of Real numbers and, therefore, the exponentiation of
    large numbers can return `inf` if it is above the
    std::numeric_limits<T>, we enforce that both the value to check and
    its target are finite.
  */
  const T tol = normalisedError<T>;
  REQUIRE(std::isfinite(value));
  REQUIRE(std::isfinite(target));

  REQUIRE_THAT(value, WithinAbs(target, tol) || WithinRel(target, tol));
}
} // namespace

TEMPLATE_TEST_CASE("Lognormal fills a buffer with expected data", "[lognormal]",
                   float, double) {
  auto method = GENERATE(VSL_RNG_METHOD_LOGNORMAL_BOXMULLER2,
                         VSL_RNG_METHOD_LOGNORMAL_ICDF);
  CAPTURE(method);
  TestType a = 0;
  TestType alpha = 1;
  TestType b = 0;
  TestType beta = 1;

  TestType absErrorBound = 0.00001;

  VSLStreamStatePtr stream;
  int errcode = vslNewStream(&stream, VSL_BRNG_MCG31, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  TestType buffer[4];
  vRngLognormal<TestType>(method, stream, 4, buffer, a, alpha, b, beta);

  auto expectedDatum = expectedData<TestType>[method];
  for (int i = 0; i < expectedDatum.size(); i++) {
    REQUIRE_THAT(buffer[i], WithinRel(expectedDatum[i], absErrorBound));
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Lognormal returns the same series when called iteratively",
                   "[lognormal]", float, double) {
  const auto n = 100;
  const auto method = GENERATE(VSL_RNG_METHOD_LOGNORMAL_ICDF,
                               VSL_RNG_METHOD_LOGNORMAL_BOXMULLER2);

  const auto mean = GENERATE(-10, 0, 1);
  const auto sigma = GENERATE(as<TestType>{}, 0.1, 1, 2);
  const auto b = GENERATE(as<TestType>{}, 1, -1, 2, -2, 123.2);
  const auto beta = GENERATE(as<TestType>{}, 1, 2);

  CAPTURE(method, mean, sigma, b, beta);

  VSLStreamStatePtr streamVector;
  VSLStreamStatePtr streamScalar;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&streamVector, VSL_BRNG_MCG31, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslNewStream(&streamScalar, VSL_BRNG_MCG31, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<TestType> buffer(n);
  errcode = vRngLognormal<TestType>(method, streamVector, buffer.size(),
                                    buffer.data(), mean, sigma, b, beta);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int i = 0; i < n; i++) {
    TestType scalar;
    errcode = vRngLognormal<TestType>(method, streamScalar, 1, &scalar, mean,
                                      sigma, b, beta);
    REQUIRE(errcode == VSL_ERROR_OK);

    check(buffer[i], scalar);
  }

  errcode = vslDeleteStream(&streamVector);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&streamScalar);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Lognormal is approximately comparable to refng",
                   "[lognormal]", float, double) {
  const auto method = GENERATE(VSL_RNG_METHOD_LOGNORMAL_BOXMULLER2,
                               VSL_RNG_METHOD_LOGNORMAL_ICDF);
  const auto skipAhead = GENERATE(0, 1, 2, 3, 100, 1000);
  const auto a = GENERATE(as<TestType>{}, -10.0, -2, 0, 1);
  const auto alpha = GENERATE(as<TestType>{}, 0.1, 1);
  const auto b = GENERATE(as<TestType>{}, -1, -2, 20);
  const auto beta = GENERATE(as<TestType>{}, 1, 2, 30);
  const auto bufferSize = GENERATE(1, 2, 3, 600);

  const int seed = GENERATE(0, 0x1234, 0x7ffffffe);

  CAPTURE(method, skipAhead, a, alpha, beta, bufferSize, seed);

  VSLStreamStatePtr vStream, rStream;
  int errcode;
  errcode = vslNewStream(&vStream, VSL_BRNG_MCG31, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refNewStream(&rStream, VSL_BRNG_MCG31, seed);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslSkipAheadStream(vStream, skipAhead);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refSkipAheadStream(rStream, skipAhead);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<TestType> vslBuffer(bufferSize);
  errcode = vRngLognormal<TestType>(method, vStream, vslBuffer.size(),
                                    vslBuffer.data(), a, alpha, b, beta);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<TestType> refBuffer(bufferSize);
  errcode = rRngLognormal<TestType>(method, rStream, refBuffer.size(),
                                    refBuffer.data(), a, alpha, b, beta);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int j = 0; j < bufferSize; j++) {
    check(vslBuffer[j], refBuffer[j]);
  }

  errcode = vslDeleteStream(&vStream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&rStream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEMPLATE_TEST_CASE("Lognormal bad arguments", "[lognormal]", float, double) {

  auto n = 100;
  auto method = GENERATE(VSL_RNG_METHOD_LOGNORMAL_BOXMULLER2,
                         VSL_RNG_METHOD_LOGNORMAL_ICDF);
  CAPTURE(method);

  TestType a = 0;
  TestType alpha = 1;
  TestType b = 0;
  TestType beta = 1;
  TestType buffer[n];

  VSLStreamStatePtr stream;
  auto errcode = VSL_ERROR_OK;

  errcode = vslNewStream(&stream, VSL_BRNG_R250, 0);
  REQUIRE(errcode == VSL_ERROR_OK);

  SECTION("Testing the first parameter (method)") {
    errcode =
        vRngLognormal<TestType>(0xffff, stream, n, buffer, a, alpha, b, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the second parameter (stream address)") {
    errcode =
        vRngLognormal<TestType>(method, nullptr, n, buffer, a, alpha, b, beta);
    REQUIRE(errcode == VSL_ERROR_NULL_PTR);
  }

  SECTION("Testing the third parameter (the size)") {
    errcode =
        vRngLognormal<TestType>(method, stream, -1, buffer, a, alpha, b, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the fourth parameter (the buffer)") {
    errcode =
        vRngLognormal<TestType>(method, stream, n, nullptr, a, alpha, b, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the sixth parameter (beta)") {
    errcode =
        vRngLognormal<TestType>(method, stream, n, buffer, a, -1, b, beta);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  SECTION("Testing the eight parameter (beta)") {
    errcode =
        vRngLognormal<TestType>(method, stream, n, buffer, a, alpha, b, -1);
    REQUIRE(errcode == VSL_ERROR_BADARGS);
  }

  errcode = vslDeleteStream(&stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}
