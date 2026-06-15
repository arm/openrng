/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <generator/qrng_base_generator.hpp>

#include <algorithm>
#include <cassert>
#include <climits>

namespace openrng {

int QRNGBaseGenerator::initialise(const Generator *baseGen) {
  if (brngId != baseGen->brngId) {
    return VSL_RNG_ERROR_BRNGS_INCOMPATIBLE;
  }

  auto gen = reinterpret_cast<const QRNGBaseGenerator *>(baseGen);

  state = gen->state;
  state_vector = gen->state_vector;
  direction_numbers = gen->direction_numbers;

  return VSL_ERROR_OK;
}

int QRNGBaseGenerator::initialise(const StateWrapperBase *stateHeader) {
  if (brngId != stateHeader->brngId) {
    return VSL_RNG_ERROR_BAD_MEM_FORMAT;
  }

  auto stateWrapper =
      reinterpret_cast<const StateWrapper<QRNGState> *>(stateHeader);

  state = stateWrapper->data;
  state_vector.resize(state.ndim);
  for (auto &dim : direction_numbers)
    dim.resize(state.ndim);

  const uint32_t *state_vector_base =
      (const uint32_t *)((const char *)stateHeader +
                         sizeof(StateWrapper<QRNGState>));
  const uint32_t *serialised_direction_numbers_base =
      state_vector_base + state.ndim;

  std::copy_n(state_vector_base, state.ndim, state_vector.data());

  for (uint32_t i = 0; i < directions_per_dimension; i++)
    std::copy_n(serialised_direction_numbers_base + i * state.ndim, state.ndim,
                direction_numbers[i].data());

  return VSL_ERROR_OK;
}

void QRNGBaseGenerator::advance() {
  /* Advance step. We keep track of position in the sequence using a
     counter, state.i. Element i of the output sequence is the result
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
     Modern Computational Finance: AAD and Parallel Simulations,
     Chapter 5.4 Antoine Savine, 2018.  */
  state.i += 1;
  const int Ji = __builtin_ctz(state.i);
  for (uint32_t i = 0; i < state.ndim; i++)
    state_vector[i] = state_vector[i] ^ direction_numbers[Ji][i];
}

int QRNGBaseGenerator::fillBits32(int64_t, uint32_t[]) {
  return VSL_RNG_ERROR_BRNG_NOT_SUPPORTED;
}

int QRNGBaseGenerator::fillBits(int64_t elemsRemaining, uint32_t dst[]) {
  /* Copy vectors to output buffer. Conceptually we just advance and copy, but
     there is a complication when the sample stops in the middle of a
     vector, or worse when the previous sample stopped in the middle of a
     vector.  */

  /* 'Head' step. Gets executed only if the previous sample stopped in the
     middle of a vector - exhaust either the current vector or the buffer
     (whichever comes first). If the buffer is exhausted first, state.offset
     is left in such a state that the next sample can pick up where
     this sample left off, i.e. state_vector is not advanced.  */
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
     current vector, and leave state.offset in such a state that the next
     sample will begin copying where we left off. */
  if (elemsRemaining != 0) {
    assert(!state.isLeapfrogged);
    advance();
    std::copy_n(state_vector.data(), elemsRemaining, dst);
    state.offset = elemsRemaining;
  }

  return VSL_ERROR_OK;
}

int QRNGBaseGenerator::fillBits64(int64_t, uint64_t[]) {
  return VSL_RNG_ERROR_BRNG_NOT_SUPPORTED;
}

int QRNGBaseGenerator::fill(int64_t n, float dst_flt[]) {
  uint32_t *dst_int = reinterpret_cast<uint32_t *>(dst_flt);
  fillBits(n, dst_int);
  for (int i = 0; i < n; i++)
    dst_flt[i] = (float)dst_int[i] / (1LL << 32);
  return VSL_ERROR_OK;
}

int QRNGBaseGenerator::fill(int64_t n, double dst_dbl[]) {
  /* Fill the second half of the buffer with 32-bit ints, then iterate forward
     from 0 and convert them.  */
  uint32_t *dst_int = reinterpret_cast<uint32_t *>(dst_dbl);
  fillBits(n, dst_int + n);
  for (int i = 0; i < n; i++)
    dst_dbl[i] = (double)dst_int[i + n] / (1LL << 32);
  return VSL_ERROR_OK;
}

int QRNGBaseGenerator::skipAhead(uint64_t n, const uint64_t params[]) {
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

int QRNGBaseGenerator::leapfrog(uint64_t choose_dim, uint64_t) {
  if (choose_dim >= state.ndim)
    return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
  state.isLeapfrogged = true;
  state.baseIndex = choose_dim;
  return VSL_ERROR_OK;
}

int QRNGBaseGenerator::stateSize() {
  /* State is serialised by writing the entire state struct, followed by
     serialised contents of state vector and direction number table, which
     are dynamically sized.  */
  return sizeof(StateWrapper<QRNGState>)
         /* Size of state vector.  */
         + state.ndim * sizeof(state_vector[0])
         /* Size of direction number table. */
         + state.ndim * directions_per_dimension * sizeof(uint32_t);
}

int QRNGBaseGenerator::save(char buffer[]) {
  /* Write State struct, as well as the state vector & direction number table -
     need to be careful to reallocate a new vector and copy the contents
     of the saved vector to it on reinitialisation.  */
  new (buffer) StateWrapper<QRNGState>(brngId, state);
  uint32_t *state_vector_base =
      (uint32_t *)(buffer + sizeof(StateWrapper<QRNGState>));
  uint32_t *dir_nums_base = state_vector_base + state.ndim;
  std::copy_n(state_vector.data(), state.ndim, state_vector_base);
  for (int i = 0; i < directions_per_dimension; i++)
    std::copy_n(direction_numbers[i].data(), state.ndim,
                dir_nums_base + i * state.ndim);

  return VSL_ERROR_OK;
}

} // namespace openrng
