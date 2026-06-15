/*
 * SPDX-FileCopyrightText: <text>Copyright 2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/generator.hpp>
#include <utils/dynamic_array.hpp>

#include <array>

namespace openrng {

struct QRNGState {
  /* Counter for the output sequence.  */
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

class QRNGBaseGenerator : public Generator {
  void advance();

protected:
  QRNGState state;

  constexpr static int directions_per_dimension = 32;

  /* Some aspects of state have dynamic size, dictated by the number of
     dimensions. These cannot be blindly copied in save/restore methods as they
     are dynamically allocated and contain a pointer. This has to be specially
     handled - store dynamic arrays directly in the Generator rather than the
     State struct to prevent issues from allocation during save and restore.  */
  DynamicArray<uint32_t> state_vector{};
  std::array<DynamicArray<uint32_t>, directions_per_dimension>
      direction_numbers{};

public:
  QRNGBaseGenerator(int64_t brng) : Generator{brng}, state{} {}

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
