/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <utils/expected.hpp>

#include <cstdint>
#include <openrng.h>

namespace openrng {

/**
 * Header of serialised state.
 */
struct StateWrapperBase {

  /**
   * Each packet should contain the git hash used to build the library. This
   * can help mitigate against garbage passed in by the user, or to avoid the
   * mixing of packets from incompatible versions of the library. The git hash
   * is quite restrictive, but we don't envisage people mixing and matching
   * library versions.
   */
  uint64_t hash = GIT_HASH;

  /**
   * The BRNG ID identifies what deserialisation scheme to use.
   */
  int64_t brngId;

  StateWrapperBase(int64_t _brngId) : brngId{_brngId} {}
  StateWrapperBase() = delete;
  StateWrapperBase(const StateWrapperBase &) = delete;
  StateWrapperBase &operator=(const StateWrapperBase &) = delete;

  /**
   * Returns whether the current StateHeader is valid.
   *
   * A valid StateHeader is defined by the presence of the library's GIT_HASH.
   */
  bool isValid() const { return hash == GIT_HASH; }

  /**
   * Extracts StateWrapperBase* from a const char *.
   *
   * Returns {nullptr, errcode} on error, {StateWrapperBase, VSL_ERROR_OK}
   * otherwise. Possible errcodes are VSL_ERROR_NULL_PTR, if the stream is NULL,
   * or VSL_RNG_ERROR_BAD_MEM_FORMAT is isValid returns false.
   */
  static Expected<const StateWrapperBase> extract(const char *memptr) {
    if (memptr == nullptr) {
      return {nullptr, VSL_ERROR_NULL_PTR};
    }

    auto stateHeader = reinterpret_cast<const StateWrapperBase *>(memptr);

    if (!stateHeader->isValid()) {
      return {nullptr, VSL_RNG_ERROR_BAD_MEM_FORMAT};
    }

    return {stateHeader, VSL_ERROR_OK};
  }
};

template <typename T> struct StateWrapper : StateWrapperBase {
  T data;

  StateWrapper(int64_t _brngId, const T &_data)
      : StateWrapperBase{_brngId}, data{_data} {}
};

} // namespace openrng
