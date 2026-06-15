/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/niederr.hpp>
#include <generator/niederr_default_initialization.hpp>

#include <algorithm>
#include <cassert>

namespace openrng {

namespace {

struct Poly {
  /* b stores powers PX**J while CALCV builds 32 direction numbers, so the
     degree can grow up to calcc2_maxv. */
  uint8_t c[NiederrGenerator::calcc2_maxv + 1] = {};
  uint32_t deg = 0;
};

Poly plymul(const Poly &pa, const Poly &pb) {
  Poly res;
  res.deg = pa.deg + pb.deg;
  for (uint32_t i = 0; i <= pa.deg; i++)
    for (uint32_t j = 0; j <= pb.deg; j++)
      res.c[i + j] ^= static_cast<uint8_t>(pa.c[i] & pb.c[j]);
  return res;
}

/* Unpack an irreducible polynomial (encoded as a bit pattern with bit i =
   coefficient of x**i) into Poly form.  */
Poly unpack_irreducible(uint32_t px) {
  Poly p;
  if (px == 0) {
    p.deg = 0;
    return p;
  }

  p.deg = 31u - static_cast<uint32_t>(__builtin_clz(px));
  for (uint32_t i = 0; i <= p.deg; i++)
    p.c[i] = (px >> i) & 1u;
  return p;
}

/* Calculation of coeffiecients v_l as per section 2.3 of BFN,
   with the adjustment of section 3.3 for the points v_l of
   BFN to avoid the leading-zeros phenomenon.
 */
void calcv(const Poly &px, Poly &b, uint8_t v[]) {

  const uint32_t bigm = b.deg;

  /* b = PX**J.  */
  b = plymul(px, b);
  const uint32_t m = b.deg;

  for (uint32_t r = 0; r < bigm; r++)
    v[r] = 0;
  v[bigm] = 1;

  /* This is taken from section 3.3 from the paper BFN, Implementation and Tests
    of Low-Discrepancy Sequences, ACM Transactions on modeling and Computer
    Simulation, Vol2, No 3, 1992, 195-213*/
  for (uint32_t r = bigm + 1; r < m; r++)
    v[r] = 1;

  /* Recurrence of BFN section 2.3
      v[r+m] = SUM_{i=0..m-1} b[i] * v[r+i]   (all in GF(2))
   */
  for (uint32_t r = 0; r + m <= NiederrGenerator::calcc2_maxv; r++) {
    uint8_t term = 0;
    for (uint32_t i = 0; i < m; i++)
      term ^= static_cast<uint8_t>(b.c[i] & v[r + i]);
    v[r + m] = term;
  }
}

} // namespace

/* This function implements the algorithm of section 2.3, with the adjustment
of section 3.3 for the points v_l of ACM Algorithm 738, Bratley, Fox &
Niederreiter (BFN) to avoid the leading-zeros phenomenon.*/
int NiederrGenerator::initialise_direction_nums(const uint32_t *polynomials) {

  for (uint32_t dim = 0; dim < state.ndim; dim++) {
    const Poly px = unpack_irreducible(polynomials[dim]);
    const uint32_t e = px.deg;
    if (e < niederr_default_initialization::mindeg) {
      return VSL_ERROR_BADARGS;
    }

    /* B starts as PX**0 = 1 (degree 0).  */
    Poly b;
    b.c[0] = 1;
    b.deg = 0;

    uint8_t v[NiederrGenerator::calcc2_maxv + 1] = {};
    uint8_t ci[directions_per_dimension][directions_per_dimension] = {};

    uint32_t u = 0;
    for (uint32_t j = 0; j < directions_per_dimension; j++) {
      if (u == 0)
        calcv(px, b, v);
      for (uint32_t r = 0; r < directions_per_dimension; r++)
        ci[j][r] = v[r + u];
      u++;
      if (u == e)
        u = 0;
    }

    /* Pack ci into the direction_numbers table. C(dim, 1, r) becomes the
       MSB of direction_numbers[r][dim] (bit directions_per_dimension-1), C(dim,
       directions_per_dimension, r) becomes bit 0.*/
    for (uint32_t r = 0; r < directions_per_dimension; r++) {
      uint32_t term = 0;
      for (uint32_t j = 0; j < directions_per_dimension; j++)
        term = (term << 1) | ci[j][r];
      direction_numbers[r][dim] = term;
    }
  }
  return VSL_ERROR_OK;
}

int NiederrGenerator::initialise(const int64_t n, const unsigned int params[]) {
  state.i = 0;
  state.offset = 0;

  state.ndim = params[0];
  bool wrong_dimensions = false;
  if (state.ndim < 1 || state.ndim > max_default_dimensions) {
    state.ndim = 1;
    wrong_dimensions = true;
  }

  uint32_t init_vals_subclass = 0;

#if defined(OPENRNG_ILP64)
  if (n != 1 && n != 2) {
#else
  if (n != 1) {
#endif
    init_vals_subclass = params[2];
    assert(params[1] == VSL_USER_QRNG_INITIAL_VALUES);
  }

  bool user_supplied_polys = false;
  bool user_supplied_dir_nums = false;
  bool use_default_initialize = true;

  if (init_vals_subclass) {
    user_supplied_polys = init_vals_subclass & VSL_USER_IRRED_POLYMS;
    user_supplied_dir_nums = init_vals_subclass & VSL_USER_DIRECTION_NUMBERS;
    use_default_initialize =
        wrong_dimensions || !(user_supplied_polys ^ user_supplied_dir_nums);
  }

  state_vector.resize(state.ndim);
  std::fill_n(state_vector.data(), state.ndim, 0u);
  for (auto &dim : direction_numbers)
    dim.resize(state.ndim);
  const uint32_t *polynomials = niederr_default_initialization::polynomials;

  if (use_default_initialize || user_supplied_polys) {
    if (user_supplied_polys && !user_supplied_dir_nums) {
      polynomials = &params[3];
    }

    const int err = initialise_direction_nums(polynomials);
    if (err != VSL_ERROR_OK) {
      direction_numbers = {};
      state_vector = {};
      return err;
    }
    return VSL_ERROR_OK;
  }

  if (user_supplied_dir_nums) {
    for (int i = 0; i < directions_per_dimension; i++)
      for (uint32_t dim = 0; dim < state.ndim; dim++)
        direction_numbers[i][dim] =
            params[3 + dim * directions_per_dimension + i];
  }

  return VSL_ERROR_OK;
}

} // namespace openrng
