/*
 * SPDX-FileCopyrightText: <text>Copyright 2024, 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/sobol.hpp>
#include <generator/sobol_default_initialization.hpp>

#include <cassert>

namespace openrng {

int SobolGenerator::initialise_direction_nums(const uint32_t *polynomials,
                                              const uint32_t *init,
                                              const uint32_t maxdeg,
                                              bool override_first_dim) {
  if (!override_first_dim)
    /* First dimension is special-case mentioned in B&F section 3.1.  */
    for (int i = 0; i < directions_per_dimension; i++)
      direction_numbers[i][0] = 1;

  const uint32_t first_dim_optional_offset = override_first_dim ? 0 : 1;
  /* The following implements the algorithm described in B&F section 2. The
     names m, d, a and v refer to the same variables as in that chapter.  */
  for (uint32_t dim = first_dim_optional_offset; dim < state.ndim; dim++) {
    /* Each dimension (except the first) has a polynomial with order d,
       and d initial values of m.  */
    const uint32_t poly = polynomials[dim - first_dim_optional_offset];
    const uint32_t *init_dirs =
        &init[(dim - first_dim_optional_offset) * maxdeg];

    const auto m = [dim, this](uint32_t i) -> uint32_t & {
      return direction_numbers[i][dim];
    };

    const uint32_t d = 31 - __builtin_clz(poly);
    if (d > maxdeg)
      return VSL_ERROR_BADARGS;

    /* Coefficient n of the polynomial.  */
    const auto a = [d, poly](uint32_t n) {
      /* If n is 0 or d, the indexing in get_m has gone wrong.  */
      assert(n > 0 && n < d);
      return ((poly >> (d - n)) & 1) == 0 ? 0 : 1;
    };

    /* Calculate m_i using the recurrence relation.  */
    const auto get_m = [d, &m, &a](uint32_t i) {
      uint32_t m_i = m(i - d) ^ (m(i - d) << d);
      for (uint32_t k = 1; k < d; k++)
        m_i ^= ((a(k) * m(i - k)) << k);
      return m_i;
    };

    uint32_t i = 0;
    /* Fill in m - first copy the initial direction numbers.  */
    for (; i < d; i++)
      m(i) = init_dirs[i];
    /* Then calculate the rest using the recurrence relation.  */
    for (; i < directions_per_dimension; i++)
      m(i) = get_m(i);
  }

  /* Transform a_i to v_i. In B&F, v is defined as
     v_i = m_i / (2**i)
     i.e. some fraction with a power of 2 as the denominator. This is
     represented using fixed-point format, where the point is between
     bits 31 and 32. The numbers are stored as 32-bit integers in the
     generator itself, so the point is to the left of the MSB.  */
  for (uint32_t dim = 0; dim < state.ndim; dim++)
    for (uint32_t i = 0; i < directions_per_dimension; i++)
      direction_numbers[i][dim] = direction_numbers[i][dim] << (31 - i);

  return VSL_ERROR_OK;
}

int SobolGenerator::initialise(const int64_t n, const unsigned int params[]) {
  state.i = 0;
  state.offset = 0;

  state.ndim = params[0];

  const uint32_t *polynomials = sobol_default_initialization::polynomials;
  /* Reinterpret as 1D array. maxdeg is used to control the stride.  */
  const uint32_t *init_dir_nums =
      (uint32_t *)sobol_default_initialization::init;
  uint32_t maxdeg = sobol_default_initialization::maxdeg;
  uint32_t init_vals_subclass = 0;

#if defined(OPENRNG_ILP64)
  if (n == 1 || n == 2) {
#else
  if (n == 1) {
#endif
    if (state.ndim < 1 || state.ndim > max_default_dimensions)
      state.ndim = 1;
  } else {
    init_vals_subclass = params[2];
    assert(params[1] == VSL_USER_QRNG_INITIAL_VALUES);
  }

  bool user_supplied_init_dir_nums = false;
  bool user_supplied_polys = false;
  bool user_supplied_dir_nums = false;
  bool override_first_dim = false;

  if (init_vals_subclass) {
    user_supplied_init_dir_nums =
        init_vals_subclass & VSL_USER_INIT_DIRECTION_NUMBERS;
    user_supplied_polys = init_vals_subclass & VSL_USER_PRIMITIVE_POLYMS;
    user_supplied_dir_nums = init_vals_subclass & VSL_USER_DIRECTION_NUMBERS;
    /* VSL_QRNG_OVERRIDE_1ST_DIM_INIT is ignored unless both
       polynomials and initial direction numbers are passed.  */
    override_first_dim =
        (init_vals_subclass & VSL_QRNG_OVERRIDE_1ST_DIM_INIT) &&
        user_supplied_polys && user_supplied_init_dir_nums;
  }

  if (state.ndim > max_default_dimensions &&
      !(user_supplied_init_dir_nums && user_supplied_polys))
    return VSL_ERROR_BADARGS;

  state_vector.resize(state.ndim);
  for (auto &dim : direction_numbers)
    dim.resize(state.ndim);

  if (user_supplied_dir_nums) {
    /* Transpose user's direction number. VSL_QRNG_OVERRIDE_1ST_DIM_INIT is
       implicitly for this option.  */
    for (int i = 0; i < directions_per_dimension; i++)
      for (uint32_t dim = 0; dim < state.ndim; dim++)
        direction_numbers[i][dim] =
            params[3 + dim * directions_per_dimension + i];
    return VSL_ERROR_OK;
  }

  if (user_supplied_polys)
    polynomials = &params[3];

  if (user_supplied_init_dir_nums) {
    if (user_supplied_polys) {
      if (override_first_dim) {
        maxdeg = params[3 + state.ndim];
        init_dir_nums = &params[4 + state.ndim];
      } else {
        maxdeg = params[2 + state.ndim];
        init_dir_nums = &params[3 + state.ndim];
      }
    } else {
      maxdeg = params[3];
      init_dir_nums = &params[4];
    }
  }

  const int errcode = initialise_direction_nums(polynomials, init_dir_nums,
                                                maxdeg, override_first_dim);
  if (errcode != VSL_ERROR_OK) {
    /* If initialising direction nums failed, direction nums array may still
       have been allocated. Deallocate before exiting.  */
    direction_numbers = {};
    state_vector = {};
    return errcode;
  }

  return VSL_ERROR_OK;
}

} // namespace openrng
