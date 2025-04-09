/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <array>
#include <cstddef>

namespace openrng {

/**
 * This class represents a fixed-size ring buffer.
 *
 * @tparam T  underlying element type
 * @tparam N  number of elements in the buffer
 */
template <typename T, size_t N> class ring_buffer {
  std::array<T, N> buffer = {};
  size_t position = {0};

public:
  ring_buffer() = default;

  /**
   * Initialise the buffer with the elements of a std::array.
   */
  ring_buffer(const std::array<T, N> arr) { buffer = arr; }

  /**
   * Access the element located `i` elements prior to the buffer end.
   */
  T &operator[](size_t i) {
    auto index = position >= i ? position - i : position + N - i;
    return buffer[index];
  }

  /**
   * Access the element located `i` elements prior to the buffer end.
   */
  const T &operator[](size_t i) const {
    auto index = position >= i ? position - i : position + N - i;
    return buffer[index];
  }

  /**
   * Return a pointer to the underlying data.
   */
  T *data() { return buffer.data(); }

  /**
   * Return the position of the first element in the underlying data.
   */
  size_t get_position() { return position; }

  /**
   * Insert an element at the end of the buffer.
   */
  void push_back(const T &val) {
    buffer[position++] = val;
    if (position >= N) {
      position = 0;
    }
  }
};

} // namespace openrng
