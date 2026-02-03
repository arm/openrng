/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include "timer.hpp"
#include "vsl_error.hpp"

#include <openrng.h>

#include <algorithm>
#include <cfloat>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace openrng::bench {

template <typename BufType, typename... Params, typename... Args>
auto distribution(int (*func)(openrng_int_t, VSLStreamStatePtr, openrng_int_t,
                              BufType *, Params...),
                  uint64_t niters, int brng, int method, int nelems,
                  int elemSize, Args &&...args) {

  VSLStreamStatePtr stream;
  VSLBRngProperties brngProperties;
  auto errcode = vslNewStream(&stream, brng, 0);
  check_no_error(errcode);

  errcode = vslGetBrngProperties(brng, &brngProperties);
  check_no_error(errcode);

  /*
    The buffer below is going to be right size for most generators
    but in the case of mcg59 it will be twice the necessary size in all cases
    except for when using func=uniformbits.

    This is a temporary fix to fix the SEGFAULT. Moving forward
    we probably want to update distributions.json and generate_bench.py to
    adjust the bufferSize specifically only when uniformbits is used.
  */
  const int bufferSize =
      brngProperties.WordSize / sizeof(uint32_t) * nelems * elemSize;

  std::vector<BufType> buffer(bufferSize);

  errcode = VSL_ERROR_OK;
  auto t = timer_start();
  for (uint64_t i = 0; i < niters; i++) {
    errcode |= func(method, stream, nelems, buffer.data(),
                    std::forward<Args>(args)...);
  }
  auto total_time = timer_end(std::move(t));
  auto time_per_iter = total_time / niters;

  check_no_error(errcode);

  vslDeleteStream(&stream);
  return time_per_iter;
}

auto skipahead(uint64_t niters, int brng, uint64_t nskip) {
  VSLStreamStatePtr stream;
  double min_time = DBL_MAX;
  for (uint64_t i = 0; i < niters; i++) {
    vslNewStream(&stream, brng, 0);

    auto t = timer_start();
    vslSkipAheadStream(stream, nskip);
    auto diff = timer_end(std::move(t));
    min_time = std::min(min_time, diff);

    vslDeleteStream(&stream);
  }
  return min_time;
}

} // namespace openrng::bench
