/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/generator.hpp>
#include <generator/sobol_default_direction_numbers.hpp>
#include <utils/dynamic_array.hpp>

namespace openrng {

struct SobolState {
  /* Counter for the Sobol sequence.  */
  uint32_t i = 0;
  /* Number of dimensions of the output vector.  */
  uint32_t ndim = 0;
  /* Used for when samples stop in the middle of a vector.  */
  uint32_t offset = 0;
  /* Store whether the stream is leapfrogged in order to be able to count output
     elements and adjust offsets properly.  */
  bool isLeapfrogged = false;
  /* Base of the state vector - adjusted in the event of a leapfrog.  */
  uint32_t baseIndex = 0;
};

class SobolGenerator : public Generator {
  void advance();

  SobolState state;

  /* State vector cannot be blindly copied in save/restore methods as it is
     dynamically allocated and contains a pointer. This has to be specially
     handled - state vector is stored directly in the Generator rather than the
     State struct to prevent issues from allocation during save and restore.  */
  DynamicArray<uint32_t> state_vector{};

public:
  SobolGenerator() : Generator{VSL_BRNG_SOBOL}, state{} {}

  using Generator::initialise;
  int initialise(const int64_t n, const unsigned int params[]);

  int initialise(const Generator *) final;
  int initialise(const StateWrapperBase *) final;
  int stateSize() final;
  int save(char[]) final;

  int fillBits(int64_t n, uint32_t[]) final;
  int fillBits32(int64_t n, uint32_t[]) final;
  int fillBits64(int64_t n, uint64_t[]) final;
  int fill(int64_t n, float[]) final;
  int fill(int64_t n, double[]) final;
  int leapfrog(uint64_t nstreams, uint64_t k) final;
  int skipAhead(uint64_t n, const uint64_t nskip[]) final;
};

} // namespace openrng
