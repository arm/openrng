/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <generator/generator.hpp>

namespace openrng {

/**
 * CopyableGenerator abstract class.
 *
 * All generators that need to support copy-in-place can inherit from
 * CopyableGenerator<State>. State needs to be a trivially copyable struct that
 * contains all data that uniquely identifies the state and contains no
 * pointers.
 */
template <typename State> class CopyableGenerator : public Generator {

protected:
  State state;

public:
  CopyableGenerator(int64_t _brngId) : Generator{_brngId} {}

  int initialise(const Generator *baseGen) final {
    if (this->brngId != baseGen->brngId) {
      return VSL_RNG_ERROR_BRNGS_INCOMPATIBLE;
    }

    auto gen = reinterpret_cast<const CopyableGenerator *>(baseGen);

    this->state = gen->state;
    return VSL_ERROR_OK;
  }

  int initialise(const StateWrapperBase *stateHeader) {
    if (this->brngId != stateHeader->brngId) {
      return VSL_RNG_ERROR_BAD_MEM_FORMAT;
    }

    auto stateWrapper =
        reinterpret_cast<const StateWrapper<State> *>(stateHeader);

    this->state = stateWrapper->data;
    return VSL_ERROR_OK;
  }

  int stateSize() { return sizeof(StateWrapper<State>); }

  int save(char buffer[]) {
    new (buffer) StateWrapper<State>(this->brngId, state);
    return VSL_ERROR_OK;
  }
};
} // namespace openrng