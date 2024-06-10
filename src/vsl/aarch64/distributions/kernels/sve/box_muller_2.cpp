/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <distributions/kernels/sve/box_muller_2.hpp>
#include <math/cospi.hpp>
#include <math/log.hpp>
#include <math/sinpi.hpp>

namespace openrng::distribution::gaussian {

template <openrng::FloatingPoint T>
int BoxMuller2<CPUFeatures::sve>::fill(Generator *generator, const int64_t n,
                                       T r[], const T a, const T sigma) {
  assert(n > 0);
  assert(generator != nullptr);

  // First check if state from a previous call has been stored inside the
  // generator. If so, we use the second term of Box Muller 2.
  const auto previousState = generator->getAndResetBoxMuller2State<T>();
  uint64_t valuesRemaining = n;
  if (previousState) {
    const auto [u1, u2] = *previousState;
    r[0] = sigma * std::sqrt(-2 * math::log(u1)) * math::cospi(2 * u2) + a;
    valuesRemaining -= 1;
  }

  // If the user only requested one element, and there had been state saved,
  // then we can exit early.
  if (valuesRemaining == 0) {
    return VSL_ERROR_OK;
  }

  // We're guaranteed &r[0] is safe because n > 0. If there's previous state
  // and we got here, then n > 1 meaning &r[1] is safe.
  T *const adjustedBase = previousState ? &r[1] : &r[0];
  const uint64_t evenValuesRemaining = valuesRemaining & ~1;
  const bool needOneMore = valuesRemaining & 1;

  // Fill all pairs using the Box Muller 2 method.
  generator->fill(evenValuesRemaining, adjustedBase);
  for (uint64_t i = 0; i < evenValuesRemaining; i += 2) {
    const auto u1 = adjustedBase[i];
    const auto u2 = adjustedBase[i + 1];
    adjustedBase[i + 0] =
        sigma * std::sqrt(-2 * math::log(u1)) * math::sinpi(2 * u2) + a;
    adjustedBase[i + 1] =
        sigma * std::sqrt(-2 * math::log(u1)) * math::cospi(2 * u2) + a;
  }

  // If the remaining space in the array was even, all pairs fit perfectly.
  if (!needOneMore) {
    return VSL_ERROR_OK;
  }

  // If we didn't have space for the last pair, fill the remaining space in
  // the output buffer using the 1st term of Box Muller 2, and save the state
  // for later.
  T remainingBuffer[2];
  generator->fill(2, remainingBuffer);

  const auto [u1, u2] = remainingBuffer;
  r[n - 1] = sigma * std::sqrt(-2 * math::log(u1)) * math::sinpi(2 * u2) + a;
  generator->storeBoxMuller2State<T>({u1, u2});

  return VSL_ERROR_OK;
}

template int BoxMuller2<CPUFeatures::sve>::fill(Generator *, const int64_t,
                                                float[], const float,
                                                const float);
template int BoxMuller2<CPUFeatures::sve>::fill(Generator *, const int64_t,
                                                double[], const double,
                                                const double);

} // namespace openrng::distribution::gaussian
