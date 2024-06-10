/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <openrng.h>
#include <utils/concepts.hpp>
#include <utils/cpu_features.hpp>
#include <utils/state_wrapper.hpp>

#include <cassert>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <optional>
#include <type_traits>
#include <utility>

namespace openrng {

class Generator {
  // A generator is considered invalid if openrngMagicNumInstance !=
  // openrngMagicNumConstant. openrngMagicNumConstant is a compile time
  // constant, while openrngMagicNumInstance is unique to each instance. These
  // generators are passed into the user's program as void pointers, so this
  // constant helps compensate for a lack of type safety in the API.
  constexpr static inline const int openrngMagicNumConstant = 0x41524e47;
  int openrngMagicNumInstance = openrngMagicNumConstant;

  // Box Muller 2 generates pairs of numbers. If the second number is unused,
  // it's stored and returned in subsequent calls, even if the state of the
  // generator has advanced. We use a custom type rather than std::pair to
  // silence some noisy ABI warnings.
  template <typename T> struct BoxMullerState {
    T first;
    T second;
  };
  std::optional<BoxMullerState<float>> boxMuller2FloatState;
  std::optional<BoxMullerState<double>> boxMuller2DoubleState;

protected:
  Generator(int64_t _brngId) : brngId{_brngId} {}

  Generator() = delete;
  Generator(const Generator &) = delete;
  Generator &operator=(const Generator &) = delete;

public:
  /**
   * Returns whether the current rng is valid.
   *
   * A valid generator is defined by the presence of
   * Generator::openrngMagicNumConstant. This method can't be virtual, because
   * if it's not a valid generator then the vtable won't exist.
   */
  bool isValid() {
    return openrngMagicNumInstance == Generator::openrngMagicNumConstant;
  }

  /**
   * Copy in-place.
   */
  virtual int initialise(const Generator *) {
    return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
  }

  /**
   * Load from StateHeader.
   */
  virtual int initialise(const StateWrapperBase *) {
    return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
  }

  /**
   * Fills r with an implementation defined number of bits.
   *
   * Which bits are random is also implementation defined.
   */
  virtual int fillBits(int64_t, uint32_t[]) {
    return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
  }

  /**
   * Fills r with n * 32 bits, uniformly distributed bits.
   */
  virtual int fillBits32(int64_t, uint32_t[]) {
    return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
  }

  /**
   * Fills r with n * 64 bits, uniformly distributed bits.
   */
  virtual int fillBits64(int64_t, uint64_t[]) {
    return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
  }

  /**
   * Fills r with n floats, uniformly distributed on (0, 1).
   *
   * Whether or not a range is inclusive of 0, is dependent on the generator.
   */
  virtual int fill(int64_t, float[]) {
    return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
  }

  /**
   * Fills r with n doubles, uniformly distributed on (0, 1).
   *
   * Whether or not a range is inclusive of 0, is dependent on the generator.
   */
  virtual int fill(int64_t, double[]) {
    return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
  }

  /**
   * Fills r with n floats, uniformly distributed on [a, b).
   *
   * Whether or not a range is inclusive of a, is dependent on the generator.
   */
  virtual int fill([[maybe_unused]] int64_t n, [[maybe_unused]] float buffer[],
                   [[maybe_unused]] const float l,
                   [[maybe_unused]] const float u) {
    return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
  }

  /**
   * Fills r with n doubles, uniformly distributed on [a, b).
   *
   * Whether or not a range is inclusive of a, is dependent on the generator.
   */
  virtual int fill([[maybe_unused]] int64_t n, [[maybe_unused]] double buffer[],
                   [[maybe_unused]] const double l,
                   [[maybe_unused]] const double u) {
    return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
  }

  /**
   * Set how many iterations to advance per variate produced.
   *
   * The effect of calling this method multiple times is multiplicative. That is
   * succesive calls to setStride(2) and setStride(3) is equivalent to
   * setStride(6)
   */
  virtual int setStride(uint64_t) { return VSL_ERROR_FEATURE_NOT_IMPLEMENTED; }

  /*
   * For all except quasi-random generators, leapfrog advances the generator by
   * k elements, then sets the output to be strided by nstreams, such that
   * element n+1 of the output stream is actually nstreams elements further on
   * in the sequence than element n.
   */
  virtual int leapfrog(uint64_t k, uint64_t nstreams) {
    const int error = skipAhead(1, &k);
    if (error == VSL_RNG_ERROR_SKIPAHEADEX_UNSUPPORTED) {
      return VSL_RNG_ERROR_LEAPFROG_UNSUPPORTED;
    } else if (error) {
      return error;
    }
    return setStride(nstreams);
  }

  /**
   * Immediately advances the generator by a fixed amount of iterations.
   *
   * Advances the generator by
   *
   *   \sum_{i=0}^n nskip[i] * 2^(i * 64)
   *
   * iterations.
   */
  virtual int skipAhead([[maybe_unused]] uint64_t n,
                        [[maybe_unused]] const uint64_t nSkip[]) {
    return VSL_ERROR_FEATURE_NOT_IMPLEMENTED;
  }

  /**
   * Returns the number of bytes needed to store a raw dump of the state.
   *
   * Used to determine the size of buffer required by save(char[]).
   */
  virtual int stateSize() { return VSL_ERROR_FEATURE_NOT_IMPLEMENTED; }

  /**
   * Dumps the raw state to a buffer of size stateSize().
   */
  virtual int save(char[]) { return VSL_ERROR_FEATURE_NOT_IMPLEMENTED; }

  /**
   * BRNG ID.
   */
  const int64_t brngId = -1;

  /**
   * Extracts openrng::Generator from a void*.
   *
   * Returns {nullptr, errcode} on error, {openrng::Generator, VSL_ERROR_OK}
   * otherwise. Possible errcodes are VSL_ERROR_NULL_PTR, if the stream is NULL,
   * or VSL_RNG_ERROR_BAD_STREAM is isValid returns false.
   */
  static Expected<Generator> extract(void *stream) {
    if (stream == nullptr) {
      return {nullptr, VSL_ERROR_NULL_PTR};
    }

    auto generator = reinterpret_cast<openrng::Generator *>(stream);

    if (!generator->isValid()) {
      return {nullptr, VSL_RNG_ERROR_BAD_STREAM};
    }

    return {generator, VSL_ERROR_OK};
  }

  /**
   * Preserve the state of the Box Muller 2 method between calls.
   *
   * Box Muller 2 generates pairs of Gaussian distributed numbers from a pair of
   * uniformly distributed numbers u1 and u2. If the 2nd number is unused,
   * this method should be used to store the u1 and u2.
   *
   * @tparam T Either float or double.
   * @param state u1 and u2.
   */
  template <openrng::FloatingPoint T>
  void storeBoxMuller2State(BoxMullerState<T> state) {
    if constexpr (std::is_same_v<T, float>) {
      boxMuller2FloatState = state;
    } else if constexpr (std::is_same_v<T, double>) {
      boxMuller2DoubleState = state;
    }
  }

  /**
   * Retrieve and reset the state of the Box Muller 2 method between calls.
   *
   * Box Muller 2 generates pairs of Gaussian distributed numbers from a pair of
   * uniformly distributed numbers u1 and u2. If the 2nd number was unused in
   * a previous call, this method should be used to retrieve u1 and u2 and clear
   * the state. u1 and u2 are preserved even if the generator has advanced since
   * the previous call.
   *
   * @tparam T Either float or double.
   * @return u1 and u2.
   */
  template <openrng::FloatingPoint T>
  std::optional<BoxMullerState<T>> getAndResetBoxMuller2State() {
    if constexpr (std::is_same_v<T, float>) {
      auto ret = boxMuller2FloatState;
      boxMuller2FloatState.reset();
      return ret;
    } else {
      auto ret = boxMuller2DoubleState;
      boxMuller2DoubleState.reset();
      return ret;
    }
  }

  /**
   * Allocate and initialise DerivedGenerator on the heap. This class can't be
   * initialised with operator new without the C++ runtime.
   */
  template <typename DerivedGenerator, typename... Args>
  static Expected<Generator> create(Args &&...args) {

    // Due to the lack of the C++ runtime, the memory is allocated with malloc
    // and then the buffer is passed into placement new which knows how to
    // initialise the class in a preallocated buffer.
    //
    auto buffer = malloc(sizeof(DerivedGenerator));
    if (!buffer) {
      return {nullptr, VSL_ERROR_MEM_FAILURE};
    }

    auto generator = new (buffer) DerivedGenerator;

    const int error = generator->initialise(args...);
    if (error) {
      free(buffer);
      return {nullptr, error};
    }

    return {generator, VSL_ERROR_OK};
  }

  template <template <CPUFeatures> class DerivedGenerator, typename... Args>
  static Expected<Generator> choose(Args &&...args);

  /**
   * Allocate and initialise brng on the heap. This class can't be initialised
   * with operator new without the C++ runtime.
   */
  template <typename... Args>
  static Expected<Generator> create(const int64_t brng, Args &&...args);

  /**
   * Operator delete usually comes from the C++ runtime library, but we avoid
   * depending on it to keep dependencies at a minimum. A virtual destructor is
   * needed to avoid undefined behaviour when using polymorphism, but this
   * requires an operator delete. We define operator delete as a member function
   * to not pollute the global namespace. operator delete can never be called on
   * this class because it is only ever initialised with placement new.
   */
  void operator delete(void *) { assert(false); }

  /**
   * Destroy the generator.
   *
   * Calls the destructor and deallocates any allocated memory.
   */
  static int destroy(Generator *generator) {
    generator->~Generator();
    free(generator);
    return VSL_ERROR_OK;
  }

  virtual ~Generator() {}
};

} // namespace openrng
