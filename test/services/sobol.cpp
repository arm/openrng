/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <openrng.h>
#include <refng_vsl.h>
#include <supports_autogen.h>

#include <array>

TEST_CASE("SOBOL expected data, direction numbers and polynomials") {
  /* Test that SOBOL output is correct when using custom direction numbers AND
     polynomial - we use numbers obtained by Joe and Kuo using search algorithm
     from:

     S. Joe and F. Y. Kuo, Constructing Sobol sequences with better
     two-dimensional projections, SIAM J. Sci. Comput. 30, 2635-2654 (2008).

     Specifically, we use parameters for dimen = 3 from new-joe-kuo-6.21201,
     accessible at https://web.maths.unsw.edu.au/~fkuo/sobol/.  */

  const uint32_t override_first_dim =
      GENERATE(0, VSL_QRNG_OVERRIDE_1ST_DIM_INIT);
  const uint32_t num_dims = override_first_dim ? 2 : 3;

  std::vector<uint32_t> params{
      num_dims,
      /* Special flags.  */
      VSL_USER_QRNG_INITIAL_VALUES,
      VSL_USER_INIT_DIRECTION_NUMBERS | VSL_USER_PRIMITIVE_POLYMS |
          override_first_dim,
      /* Polynomials, post-processed for the required format. The Joe and Kuo
         numbers do not quite follow the format described in B&F 4.2,
         specifically the x^0 and x^d terms are not encoded (because both by
         definition of primitive polynomial are always set). The polynomials
         below are (a << 1) | (1 | 1 << s), where a is the polynomial as encoded
         in Joe & Kuo, and s is the order of a (no need to recalculate this as
         it is provided by Joe & Kuo).  */
      3,
      7,
      /* Max degree of any dimension.  */
      2,
      /* Initial direction numbers, in 'dimension-major' order and including
         zeroes where the degree of the polynomial for that dimension is less
         than the max.
         Dimension 1:  */
      1,
      0,
      /* Dimension 2:  */
      1,
      3,
  };

  VSLStreamStatePtr stream;
  int err;

  err = vslNewStreamEx(&stream, VSL_BRNG_SOBOL, params.size(), params.data());
  REQUIRE(err == VSL_ERROR_OK);

  const int nElems = 10 * num_dims;
  std::vector<float> out(nElems);
  err = vsRngUniform(VSL_RNG_METHOD_UNIFORM_STD, stream, nElems, out.data(), 0,
                     1);
  REQUIRE(err == VSL_ERROR_OK);

  /* This sequence starts one vector later than Joe and Kuo, whose first vector
     is all zeroes.  */
  const auto expected =
      override_first_dim
          ? std::vector<float>{0.5,    0.5,    0.25,   0.25,   0.75,
                               0.75,   0.375,  0.625,  0.875,  0.125,
                               0.125,  0.875,  0.625,  0.375,  0.3125,
                               0.9375, 0.8125, 0.4375, 0.0625, 0.6875}
          : std::vector<float>{0.5,    0.5,    0.5,    0.75,   0.25,   0.25,
                               0.25,   0.75,   0.75,   0.375,  0.375,  0.625,
                               0.875,  0.875,  0.125,  0.625,  0.125,  0.875,
                               0.125,  0.625,  0.375,  0.1875, 0.3125, 0.9375,
                               0.6875, 0.8125, 0.4375, 0.9375, 0.0625, 0.6875};
  REQUIRE(out == expected);

  err = vslDeleteStream(&stream);
  REQUIRE(err == VSL_ERROR_OK);
}

TEST_CASE("SOBOL reference test, Joe and Kuo, 50 dimensions") {
  const uint32_t override_first_dim =
      GENERATE(0, VSL_QRNG_OVERRIDE_1ST_DIM_INIT);
  const uint32_t num_dims = override_first_dim ? 49 : 50;
  /* Numbers taken from
     S. Joe and F. Y. Kuo, Constructing Sobol sequences with better
     two-dimensional projections, SIAM J. Sci. Comput. 30, 2635-2654 (2008).  */
  std::vector<uint32_t> params{
      num_dims,
      /* Special flags.  */
      VSL_USER_QRNG_INITIAL_VALUES,
      VSL_USER_INIT_DIRECTION_NUMBERS | VSL_USER_PRIMITIVE_POLYMS |
          override_first_dim,
      /* Polynomials, post-processed to add x^0 and x^d term as above.  */
      3, 7, 11, 13, 19, 25, 37, 41, 47, 55, 59, 61, 67, 91, 97, 103, 109, 115,
      131, 137, 143, 145, 157, 167, 171, 185, 191, 193, 203, 211, 213, 229, 239,
      241, 247, 253, 285, 299, 301, 333, 351, 355, 357, 361, 369, 391, 397, 425,
      451,
      /* Max degree of any dimension.  */
      8,
      /* Zero-padded initial direction numbers.  */
      /* Dimension 1.  */
      1, 0, 0, 0, 0, 0, 0, 0,
      /* Dimension 2.  */
      1, 3, 0, 0, 0, 0, 0, 0,
      /* Dimension 3.  */
      1, 3, 1, 0, 0, 0, 0, 0,
      /* Dimension 4.  */
      1, 1, 1, 0, 0, 0, 0, 0,
      /* Dimension 5.  */
      1, 1, 3, 3, 0, 0, 0, 0,
      /* Dimension 6.  */
      1, 3, 5, 13, 0, 0, 0, 0,
      /* Dimension 7.  */
      1, 1, 5, 5, 17, 0, 0, 0,
      /* Dimension 8.  */
      1, 1, 5, 5, 5, 0, 0, 0,
      /* Dimension 9.  */
      1, 1, 7, 11, 19, 0, 0, 0,
      /* Dimension 10.  */
      1, 1, 5, 1, 1, 0, 0, 0,
      /* Dimension 11.  */
      1, 1, 1, 3, 11, 0, 0, 0,
      /* Dimension 12.  */
      1, 3, 5, 5, 31, 0, 0, 0,
      /* Dimension 13.  */
      1, 3, 3, 9, 7, 49, 0, 0,
      /* Dimension 14.  */
      1, 1, 1, 15, 21, 21, 0, 0,
      /* Dimension 15.  */
      1, 3, 1, 13, 27, 49, 0, 0,
      /* Dimension 16.  */
      1, 1, 1, 15, 7, 5, 0, 0,
      /* Dimension 17.  */
      1, 3, 1, 15, 13, 25, 0, 0,
      /* Dimension 18.  */
      1, 1, 5, 5, 19, 61, 0, 0,
      /* Dimension 19.  */
      1, 3, 7, 11, 23, 15, 103, 0,
      /* Dimension 20.  */
      1, 3, 7, 13, 13, 15, 69, 0,
      /* Dimension 21.  */
      1, 1, 3, 13, 7, 35, 63, 0,
      /* Dimension 22.  */
      1, 3, 5, 9, 1, 25, 53, 0,
      /* Dimension 23.  */
      1, 3, 1, 13, 9, 35, 107, 0,
      /* Dimension 24.  */
      1, 3, 1, 5, 27, 61, 31, 0,
      /* Dimension 25.  */
      1, 1, 5, 11, 19, 41, 61, 0,
      /* Dimension 26.  */
      1, 3, 5, 3, 3, 13, 69, 0,
      /* Dimension 27.  */
      1, 1, 7, 13, 1, 19, 1, 0,
      /* Dimension 28.  */
      1, 3, 7, 5, 13, 19, 59, 0,
      /* Dimension 29.  */
      1, 1, 3, 9, 25, 29, 41, 0,
      /* Dimension 30.  */
      1, 3, 5, 13, 23, 1, 55, 0,
      /* Dimension 31.  */
      1, 3, 7, 3, 13, 59, 17, 0,
      /* Dimension 32.  */
      1, 3, 1, 3, 5, 53, 69, 0,
      /* Dimension 33.  */
      1, 1, 5, 5, 23, 33, 13, 0,
      /* Dimension 34.  */
      1, 1, 7, 7, 1, 61, 123, 0,
      /* Dimension 35.  */
      1, 1, 7, 9, 13, 61, 49, 0,
      /* Dimension 36.  */
      1, 3, 3, 5, 3, 55, 33, 0,
      /* Dimension 37.  */
      1, 3, 1, 15, 31, 13, 49, 245,
      /* Dimension 38.  */
      1, 3, 5, 15, 31, 59, 63, 97,
      /* Dimension 39.  */
      1, 3, 1, 11, 11, 11, 77, 249,
      /* Dimension 40.  */
      1, 3, 1, 11, 27, 43, 71, 9,
      /* Dimension 41.  */
      1, 1, 7, 15, 21, 11, 81, 45,
      /* Dimension 42.  */
      1, 3, 7, 3, 25, 31, 65, 79,
      /* Dimension 43.  */
      1, 3, 1, 1, 19, 11, 3, 205,
      /* Dimension 44.  */
      1, 1, 5, 9, 19, 21, 29, 157,
      /* Dimension 45.  */
      1, 3, 7, 11, 1, 33, 89, 185,
      /* Dimension 46.  */
      1, 3, 3, 3, 15, 9, 79, 71,
      /* Dimension 47.  */
      1, 3, 7, 11, 15, 39, 119, 27,
      /* Dimension 48.  */
      1, 1, 3, 1, 11, 31, 97, 225,
      /* Dimension 49.  */
      1, 1, 1, 3, 23, 43, 57, 177};

  VSLStreamStatePtr vsl_stream, ref_stream;
  int errcode;

  errcode =
      vslNewStreamEx(&vsl_stream, VSL_BRNG_SOBOL, params.size(), params.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode =
      refNewStreamEx(&ref_stream, VSL_BRNG_SOBOL, params.size(), params.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  constexpr int nelems = 1000;
  std::array<unsigned int, nelems> vsl_buffer;
  std::array<unsigned int, nelems> ref_buffer;

  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, vsl_stream, nelems,
                             vsl_buffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = riRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, ref_stream, nelems,
                             ref_buffer.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  REQUIRE(vsl_buffer == ref_buffer);

  errcode = vslDeleteStream(&vsl_stream);
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = refDeleteStream(&ref_stream);
  REQUIRE(errcode == VSL_ERROR_OK);
}

TEST_CASE("SOBOL expected data, polynomials") {
  /* OVERRIDE should be ignored if passing only polynomials.  */
  const uint32_t override_first_dim =
      GENERATE(0, VSL_QRNG_OVERRIDE_1ST_DIM_INIT);
  std::vector<uint32_t> params{
      /* Number of dimensions.  */
      3,
      /* Special flags.  */
      VSL_USER_QRNG_INITIAL_VALUES,
      VSL_USER_PRIMITIVE_POLYMS | override_first_dim,
      /* Polynomials.  */
      3,
      7,
  };

  VSLStreamStatePtr stream;
  int err;

  err = vslNewStreamEx(&stream, VSL_BRNG_SOBOL, params.size(), params.data());
  REQUIRE(err == VSL_ERROR_OK);

  const int nElems = 30;
  std::vector<float> out(nElems);
  err = vsRngUniform(VSL_RNG_METHOD_UNIFORM_STD, stream, nElems, out.data(), 0,
                     1);
  REQUIRE(err == VSL_ERROR_OK);

  std::vector<float> expected{0.5,    0.5,    0.5,    0.75,   0.25,   0.75,
                              0.25,   0.75,   0.25,   0.375,  0.375,  0.625,
                              0.875,  0.875,  0.125,  0.625,  0.125,  0.375,
                              0.125,  0.625,  0.875,  0.1875, 0.3125, 0.3125,
                              0.6875, 0.8125, 0.8125, 0.9375, 0.0625, 0.5625};

  REQUIRE(out == expected);

  err = vslDeleteStream(&stream);
  REQUIRE(err == VSL_ERROR_OK);
}

TEST_CASE(
    "SOBOL expected data, polynomials - degrees cannot exceed default maxdeg",
    "[openrng_only]") {
  std::vector<uint32_t> params{
      /* Number of dimensions.  */
      3,
      /* Special flags.  */
      VSL_USER_QRNG_INITIAL_VALUES, VSL_USER_PRIMITIVE_POLYMS,
      /* Polynomials.  */
      3, 545, /* Joe & Kuo dimension 56.  */
  };

  VSLStreamStatePtr stream;
  int err;

  err = vslNewStreamEx(&stream, VSL_BRNG_SOBOL, params.size(), params.data());
  REQUIRE(err == VSL_ERROR_BADARGS);
}

TEST_CASE("SOBOL custom poly, unsupported number of dims", "[openrng_only]") {
  std::vector<uint32_t> params{
      /* Number of dimensions.  */
      41,
      /* Special flags.  */
      VSL_USER_QRNG_INITIAL_VALUES, VSL_USER_PRIMITIVE_POLYMS,
      /* 40 polynomials (requested 41 dimensions, but no override of first
         dimension.  */
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
      21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
      39};

  VSLStreamStatePtr stream;
  int err;

  err = vslNewStreamEx(&stream, VSL_BRNG_SOBOL, params.size(), params.data());
  REQUIRE(err == VSL_ERROR_BADARGS);
}

TEST_CASE("SOBOL expected data, init direction numbers", "[openrng_only]") {
  /* OVERRIDE should be ignored if passing only initial direction numbers.  */
  const uint32_t override_first_dim =
      GENERATE(0, VSL_QRNG_OVERRIDE_1ST_DIM_INIT);
  std::vector<uint32_t> params{
      /* Number of dimensions.  */
      4,
      /* Special flags.  */
      VSL_USER_QRNG_INITIAL_VALUES,
      VSL_USER_INIT_DIRECTION_NUMBERS | override_first_dim,
      /* Max degree of any dimension.  */
      3,
      /* Initial direction numbers, in 'dimension-major' order and including
         zeroes where the degree of the polynomial for that dimension is less
         than the max.  */
      /* Joe & Kuo dimension 2.  */
      1,
      3,
      0,
      /* Joe & Kuo dimension 3.  */
      1,
      3,
      1,
      /* Joe & Kuo dimension 4.  */
      1,
      1,
      1,
  };

  VSLStreamStatePtr stream;
  int err;

  err = vslNewStreamEx(&stream, VSL_BRNG_SOBOL, params.size(), params.data());
  REQUIRE(err == VSL_ERROR_OK);

  const int nElems = 40;
  std::vector<float> out(nElems);
  err = vsRngUniform(VSL_RNG_METHOD_UNIFORM_STD, stream, nElems, out.data(), 0,
                     1);
  REQUIRE(err == VSL_ERROR_OK);

  std::vector<float> expected{
      0.5f,    0.5f,    0.5f,    0.5f,    0.75f,   0.25f,   0.25f,   0.75f,
      0.25f,   0.75f,   0.75f,   0.25f,   0.375f,  0.375f,  0.625f,  0.375f,
      0.875f,  0.875f,  0.125f,  0.875f,  0.625f,  0.125f,  0.875f,  0.625f,
      0.125f,  0.625f,  0.375f,  0.125f,  0.1875f, 0.3125f, 0.9375f, 0.9375f,
      0.6875f, 0.8125f, 0.4375f, 0.4375f, 0.9375f, 0.0625f, 0.6875f, 0.1875f};
  REQUIRE(out == expected);

  err = vslDeleteStream(&stream);
  REQUIRE(err == VSL_ERROR_OK);
}

TEST_CASE("SOBOL custom direction numbers - dims 1 and 2") {
  /* Use the default direction numbers for dimension 1 and 2, and compare the
     stream against a stream generated using the default parameters.  */
  VSLStreamStatePtr stream_ref, stream_custom;
  int err;

  err = vslNewStream(&stream_ref, VSL_BRNG_SOBOL, 3);
  REQUIRE(err == VSL_ERROR_OK);

  std::vector<uint32_t> params{
      2,
      /* Special flags.  */
      VSL_USER_QRNG_INITIAL_VALUES,
      VSL_USER_DIRECTION_NUMBERS,
      /* Default params for dimension 1.  */
      0x80000000,
      0xc0000000,
      0xa0000000,
      0xf0000000,
      0x88000000,
      0xcc000000,
      0xaa000000,
      0xff000000,
      0x80800000,
      0xc0c00000,
      0xa0a00000,
      0xf0f00000,
      0x88880000,
      0xcccc0000,
      0xaaaa0000,
      0xffff0000,
      0x80008000,
      0xc000c000,
      0xa000a000,
      0xf000f000,
      0x88008800,
      0xcc00cc00,
      0xaa00aa00,
      0xff00ff00,
      0x80808080,
      0xc0c0c0c0,
      0xa0a0a0a0,
      0xf0f0f0f0,
      0x88888888,
      0xcccccccc,
      0xaaaaaaaa,
      0xffffffff,
      /* Default params for dimension 2.  */
      0x80000000,
      0x40000000,
      0xe0000000,
      0xb0000000,
      0x68000000,
      0xf4000000,
      0x86000000,
      0x4f000000,
      0xe8800000,
      0xb4400000,
      0x66e00000,
      0xffb00000,
      0x80e80000,
      0x40b40000,
      0xe0660000,
      0xb0ff0000,
      0x68808000,
      0xf4404000,
      0x86e0e000,
      0x4fb0b000,
      0xe8e86800,
      0xb4b4f400,
      0x66668600,
      0xffff4f00,
      0x80006880,
      0x4000f440,
      0xe00086e0,
      0xb0004fb0,
      0x6800e8e8,
      0xf400b4b4,
      0x86006666,
      0x4f00ffff,
  };

  err = vslNewStreamEx(&stream_custom, VSL_BRNG_SOBOL, params.size(),
                       params.data());
  REQUIRE(err == VSL_ERROR_OK);

  const int nElems = 10;
  std::vector<uint32_t> out_ref(nElems * 3);
  err = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, stream_ref, nElems * 3,
                         out_ref.data());

  REQUIRE(err == VSL_ERROR_OK);
  std::vector<uint32_t> out_custom(nElems * 2);
  err = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, stream_custom,
                         nElems * 2, out_custom.data());

  REQUIRE(err == VSL_ERROR_OK);

  for (int i = 0; i < nElems; i++) {
    REQUIRE(out_custom[i * 2] == out_ref[i * 3 + 1]);
    REQUIRE(out_custom[i * 2 + 1] == out_ref[i * 3 + 2]);
  }

  err = vslDeleteStream(&stream_ref);
  REQUIRE(err == VSL_ERROR_OK);
  err = vslDeleteStream(&stream_custom);
  REQUIRE(err == VSL_ERROR_OK);
}

TEST_CASE("SOBOL VSL_QRNG_OVERRIDE_1ST_DIM_INIT, Joe and Kuo, 50 dimensions") {
  /* Numbers taken from
     S. Joe and F. Y. Kuo, Constructing Sobol sequences with better
     two-dimensional projections, SIAM J. Sci. Comput. 30, 2635-2654 (2008).  */
  const std::vector<uint32_t> params_no_override{
      50,
      /* Special flags.  */
      VSL_USER_QRNG_INITIAL_VALUES,
      VSL_USER_INIT_DIRECTION_NUMBERS | VSL_USER_PRIMITIVE_POLYMS,
      /* Polynomials, post-processed to add x^0 and x^d term as above.  */
      3, 7, 11, 13, 19, 25, 37, 41, 47, 55, 59, 61, 67, 91, 97, 103, 109, 115,
      131, 137, 143, 145, 157, 167, 171, 185, 191, 193, 203, 211, 213, 229, 239,
      241, 247, 253, 285, 299, 301, 333, 351, 355, 357, 361, 369, 391, 397, 425,
      451,
      /* Max degree of any dimension.  */
      8,
      /* Zero-padded initial direction numbers.  */
      /* Dimension 1.  */
      1, 0, 0, 0, 0, 0, 0, 0,
      /* Dimension 2.  */
      1, 3, 0, 0, 0, 0, 0, 0,
      /* Dimension 3.  */
      1, 3, 1, 0, 0, 0, 0, 0,
      /* Dimension 4.  */
      1, 1, 1, 0, 0, 0, 0, 0,
      /* Dimension 5.  */
      1, 1, 3, 3, 0, 0, 0, 0,
      /* Dimension 6.  */
      1, 3, 5, 13, 0, 0, 0, 0,
      /* Dimension 7.  */
      1, 1, 5, 5, 17, 0, 0, 0,
      /* Dimension 8.  */
      1, 1, 5, 5, 5, 0, 0, 0,
      /* Dimension 9.  */
      1, 1, 7, 11, 19, 0, 0, 0,
      /* Dimension 10.  */
      1, 1, 5, 1, 1, 0, 0, 0,
      /* Dimension 11.  */
      1, 1, 1, 3, 11, 0, 0, 0,
      /* Dimension 12.  */
      1, 3, 5, 5, 31, 0, 0, 0,
      /* Dimension 13.  */
      1, 3, 3, 9, 7, 49, 0, 0,
      /* Dimension 14.  */
      1, 1, 1, 15, 21, 21, 0, 0,
      /* Dimension 15.  */
      1, 3, 1, 13, 27, 49, 0, 0,
      /* Dimension 16.  */
      1, 1, 1, 15, 7, 5, 0, 0,
      /* Dimension 17.  */
      1, 3, 1, 15, 13, 25, 0, 0,
      /* Dimension 18.  */
      1, 1, 5, 5, 19, 61, 0, 0,
      /* Dimension 19.  */
      1, 3, 7, 11, 23, 15, 103, 0,
      /* Dimension 20.  */
      1, 3, 7, 13, 13, 15, 69, 0,
      /* Dimension 21.  */
      1, 1, 3, 13, 7, 35, 63, 0,
      /* Dimension 22.  */
      1, 3, 5, 9, 1, 25, 53, 0,
      /* Dimension 23.  */
      1, 3, 1, 13, 9, 35, 107, 0,
      /* Dimension 24.  */
      1, 3, 1, 5, 27, 61, 31, 0,
      /* Dimension 25.  */
      1, 1, 5, 11, 19, 41, 61, 0,
      /* Dimension 26.  */
      1, 3, 5, 3, 3, 13, 69, 0,
      /* Dimension 27.  */
      1, 1, 7, 13, 1, 19, 1, 0,
      /* Dimension 28.  */
      1, 3, 7, 5, 13, 19, 59, 0,
      /* Dimension 29.  */
      1, 1, 3, 9, 25, 29, 41, 0,
      /* Dimension 30.  */
      1, 3, 5, 13, 23, 1, 55, 0,
      /* Dimension 31.  */
      1, 3, 7, 3, 13, 59, 17, 0,
      /* Dimension 32.  */
      1, 3, 1, 3, 5, 53, 69, 0,
      /* Dimension 33.  */
      1, 1, 5, 5, 23, 33, 13, 0,
      /* Dimension 34.  */
      1, 1, 7, 7, 1, 61, 123, 0,
      /* Dimension 35.  */
      1, 1, 7, 9, 13, 61, 49, 0,
      /* Dimension 36.  */
      1, 3, 3, 5, 3, 55, 33, 0,
      /* Dimension 37.  */
      1, 3, 1, 15, 31, 13, 49, 245,
      /* Dimension 38.  */
      1, 3, 5, 15, 31, 59, 63, 97,
      /* Dimension 39.  */
      1, 3, 1, 11, 11, 11, 77, 249,
      /* Dimension 40.  */
      1, 3, 1, 11, 27, 43, 71, 9,
      /* Dimension 41.  */
      1, 1, 7, 15, 21, 11, 81, 45,
      /* Dimension 42.  */
      1, 3, 7, 3, 25, 31, 65, 79,
      /* Dimension 43.  */
      1, 3, 1, 1, 19, 11, 3, 205,
      /* Dimension 44.  */
      1, 1, 5, 9, 19, 21, 29, 157,
      /* Dimension 45.  */
      1, 3, 7, 11, 1, 33, 89, 185,
      /* Dimension 46.  */
      1, 3, 3, 3, 15, 9, 79, 71,
      /* Dimension 47.  */
      1, 3, 7, 11, 15, 39, 119, 27,
      /* Dimension 48.  */
      1, 1, 3, 1, 11, 31, 97, 225,
      /* Dimension 49.  */
      1, 1, 1, 3, 23, 43, 57, 177};

  int errcode;
  constexpr int num_vectors = 20;

  VSLStreamStatePtr stream_no_override;
  errcode =
      vslNewStreamEx(&stream_no_override, VSL_BRNG_SOBOL,
                     params_no_override.size(), params_no_override.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::array<unsigned int, num_vectors * 50> buffer_no_override;
  errcode =
      viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD, stream_no_override,
                       buffer_no_override.size(), buffer_no_override.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&stream_no_override);
  REQUIRE(errcode == VSL_ERROR_OK);

  std::vector<uint32_t> params_with_override = params_no_override;
  /* Override means one fewer dimensions.  */
  params_with_override[0]--;
  params_with_override[2] |= VSL_QRNG_OVERRIDE_1ST_DIM_INIT;

  VSLStreamStatePtr stream_with_override;
  errcode =
      vslNewStreamEx(&stream_with_override, VSL_BRNG_SOBOL,
                     params_with_override.size(), params_with_override.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  std::array<unsigned int, num_vectors * 49> buffer_with_override;
  errcode = viRngUniformBits(VSL_RNG_METHOD_UNIFORMBITS_STD,
                             stream_with_override, buffer_with_override.size(),
                             buffer_with_override.data());
  REQUIRE(errcode == VSL_ERROR_OK);

  errcode = vslDeleteStream(&stream_with_override);
  REQUIRE(errcode == VSL_ERROR_OK);

  for (int v = 0; v < num_vectors; v++)
    for (int i = 0; i < 49; i++)
      REQUIRE(buffer_with_override[v * 49 + i] ==
              buffer_no_override[v * 50 + i + 1]);
}
