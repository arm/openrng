/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/sobol.hpp>
#include <generator/sobol_default_initialization.hpp>

#include <climits>
#include <cstring>

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

int SobolGenerator::initialise(const Generator *baseGen) {
  if (brngId != baseGen->brngId) {
    return VSL_RNG_ERROR_BRNGS_INCOMPATIBLE;
  }

  auto gen = reinterpret_cast<const SobolGenerator *>(baseGen);

  state = gen->state;
  state_vector = gen->state_vector;
  direction_numbers = gen->direction_numbers;

  return VSL_ERROR_OK;
}

int SobolGenerator::initialise(const StateWrapperBase *stateHeader) {
  if (brngId != stateHeader->brngId) {
    return VSL_RNG_ERROR_BAD_MEM_FORMAT;
  }

  auto stateWrapper =
      reinterpret_cast<const StateWrapper<SobolState> *>(stateHeader);

  state = stateWrapper->data;
  state_vector.resize(state.ndim);
  for (auto &dim : direction_numbers)
    dim.resize(state.ndim);

  const uint32_t *state_vector_base =
      (const uint32_t *)((const char *)stateHeader +
                         sizeof(StateWrapper<SobolState>));
  const uint32_t *serialised_direction_numbers_base =
      state_vector_base + state.ndim;

  std::copy_n(state_vector_base, state.ndim, state_vector.data());

  for (uint32_t i = 0; i < directions_per_dimension; i++)
    std::copy_n(serialised_direction_numbers_base + i * state.ndim, state.ndim,
                direction_numbers[i].data());

  return VSL_ERROR_OK;
}

void SobolGenerator::advance() {
  /* Advance step. We keep track of position in the sequence using a
     counter, state.i. Element i of the Sobol sequence is the result
     of XOR-ing together direction numbers v_j, for all j for which
     bit j is 1 in the Gray code representation of i.

     Because only one bit of the Gray code changes on each increment,
     and the index of the bit that differs between GC(i) and GC(i - 1)
     is the index of the right-most zero bit in i, this can be defined
     inductively as

     x_i = x_{i-1} ^ v_{Ji}

     where Ji is the index of the right-most zero bit in i-1
     (equivalently the number of trailing zeroes in i).
     This is explained very nicely in
     Modern Computational Finance: AAD and Parallel Simulations, Chapter 5.4
     Antoine Savine, 2018.  */
  state.i += 1;
  const int Ji = __builtin_ctz(state.i);
  for (uint32_t i = 0; i < state.ndim; i++)
    state_vector[i] = state_vector[i] ^ direction_numbers[Ji][i];
}

int SobolGenerator::fillBits32(int64_t, uint32_t[]) {
  return VSL_RNG_ERROR_BRNG_NOT_SUPPORTED;
}

int SobolGenerator::fillBits(int64_t elemsRemaining, uint32_t dst[]) {
  /* Copy vectors to output buffer. Conceptually we just advance and copy, but
     there is a complication when the sample stops in the middle of a vector, or
     worse when the previous sample stopped in the middle of a vector.  */

  /* 'Head' step. Gets executed only if the previous sample stopped in the
     middle of a vector - exhaust either the current vector or the buffer
     (whichever comes first). If the buffer is exhausted first, state.offset is
     left in such a state that the next sample can pick up where this sample
     left off, i.e. state_vector is not advanced.  */
  while (state.offset != 0 && elemsRemaining > 0) {
    if (!state.isLeapfrogged || state.offset == state.baseIndex) {
      dst[0] = state_vector[state.offset];
      dst++;
      elemsRemaining--;
    }
    state.offset = (state.offset + 1) % state.ndim;
  }

  /* 'Body' step - while there are still full vectors to go, just keep advancing
     and copying.  */
  const int elemsPerAdvance = state.isLeapfrogged ? 1 : state.ndim;
  while (elemsRemaining >= elemsPerAdvance) {
    advance();
    std::copy_n(&state_vector[state.baseIndex], elemsPerAdvance, dst);
    dst += elemsPerAdvance;
    elemsRemaining -= elemsPerAdvance;
  }

  /* If there were any remaining elements, then we need to copy only part of the
     current vector, and leave state.offset in such a state that the next sample
     will begin copying where we left off. */
  if (elemsRemaining != 0) {
    assert(!state.isLeapfrogged);
    advance();
    std::copy_n(state_vector.data(), elemsRemaining, dst);
    state.offset = elemsRemaining;
  }

  return VSL_ERROR_OK;
}

int SobolGenerator::fillBits64(int64_t, uint64_t[]) {
  return VSL_RNG_ERROR_BRNG_NOT_SUPPORTED;
}

int SobolGenerator::fill(int64_t n, float dst_flt[]) {
  uint32_t *dst_int = reinterpret_cast<uint32_t *>(dst_flt);
  fillBits(n, dst_int);
  for (int i = 0; i < n; i++)
    dst_flt[i] = (float)dst_int[i] / (1LL << 32);
  return VSL_ERROR_OK;
}

int SobolGenerator::fill(int64_t n, double dst_dbl[]) {
  /* Fill the second half of the buffer with 32-bit ints, then iterate forward
     from 0 and convert them.  */
  uint32_t *dst_int = reinterpret_cast<uint32_t *>(dst_dbl);
  fillBits(n, dst_int + n);
  for (int i = 0; i < n; i++)
    dst_dbl[i] = (double)dst_int[i + n] / (1LL << 32);
  return VSL_ERROR_OK;
}

int SobolGenerator::skipAhead(uint64_t n, const uint64_t params[]) {
  if (n != 1)
    return VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED;

  uint32_t nskip = params[0];

  if ((uint64_t)nskip + state.i >= UINT_MAX) {
    /* Skipping past the end of the sequence is not supported.  */
    return VSL_RNG_ERROR_SKIPAHEAD_UNSUPPORTED;
  }

  /* Head step - exhaust either the current vector or nskips.  */
  while (state.offset != 0 && nskip > 0) {
    state.offset = (state.offset + 1) % state.ndim;
    nskip--;
  }

  /* Body step - there are full vectors to skip past. Instead of advancing
     past them, we can add numFullVecs to i, then recalculate state vector
     using the Gray code representation.  */
  if (nskip >= state.ndim) {
    const int numFullVecs = nskip / state.ndim; // NOLINT
    state.i += numFullVecs;
    nskip -= numFullVecs * state.ndim;

    /* Recalculate state vector. First, zero it.  */
    std::fill_n(state_vector.data(), state.ndim, 0);
    /* Get Gray code representation of counter.  */
    uint32_t gray = state.i ^ (state.i >> 1);
    /* Iterate over bits of Gray code - if bit i is set then XOR in the
       corresponding direction number.  */
    for (int i = 0; gray; i++, gray >>= 1) {
      if (gray & 1) {
        for (uint32_t dim = 0; dim < state.ndim; dim++) {
          state_vector[dim] ^= direction_numbers[i][dim];
        }
      }
    }
  }

  /* Tail step if there are still elements to skip - there won't be sufficient
     to skip a whole vector. Do a normal advance and update offset.  */
  if (nskip != 0) {
    advance();
    state.offset = nskip;
  }

  return VSL_ERROR_OK;
}

int SobolGenerator::leapfrog(uint64_t choose_dim, uint64_t) {
  if (choose_dim >= state.ndim)
    return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
  state.isLeapfrogged = true;
  state.baseIndex = choose_dim;
  return VSL_ERROR_OK;
}

int SobolGenerator::stateSize() {
  /* State is serialised by writing the entire state struct, followed by
     serialised contents of state vector and direction number table, which are
     dynamically sized.  */
  return sizeof(StateWrapper<SobolState>)
         /* Size of state vector.  */
         + state.ndim * sizeof(state_vector[0])
         /* Size of direction number table. */
         + state.ndim * directions_per_dimension * sizeof(uint32_t);
}

int SobolGenerator::save(char buffer[]) {
  /* Write State struct, as well as the state vector & direction number table -
     need to be careful to reallocate a new vector and copy the contents of the
     saved vector to it on reinitialisation.  */
  new (buffer) StateWrapper<SobolState>(brngId, state);
  uint32_t *state_vector_base =
      (uint32_t *)(buffer + sizeof(StateWrapper<SobolState>));
  uint32_t *dir_nums_base = state_vector_base + state.ndim;
  std::copy_n(state_vector.data(), state.ndim, state_vector_base);
  for (int i = 0; i < directions_per_dimension; i++)
    std::copy_n(direction_numbers[i].data(), state.ndim,
                dir_nums_base + i * state.ndim);

  return VSL_ERROR_OK;
}
} // namespace openrng
