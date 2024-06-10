/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>

/**
 * Dynamically sized array.
 *
 * Very basic wrapper for creating and managing dynamically sized arrays.
 */
template <typename T> struct DynamicArray {
private:
  T *_data = nullptr;
  size_t _size = 0;

public:
  DynamicArray() {}

  DynamicArray(size_t n)
      : _data{static_cast<T *>(malloc(n * sizeof(T)))}, _size{n} {}

  DynamicArray(size_t n, T v) : DynamicArray{n} {
    std::fill(_data, _data + n, v);
  }

  DynamicArray(const DynamicArray &other) { *this = other; }

  DynamicArray(DynamicArray &&other) { *this = std::move(other); }

  DynamicArray &operator=(const DynamicArray &other) {
    if (&other == this) {
      return *this;
    }
    *this = DynamicArray<T>(other._size);
    std::copy(other._data, other._data + other._size, _data);
    return *this;
  }

  DynamicArray &operator=(DynamicArray &&other) {
    _size = other._size;
    std::swap(_data, other._data);
    return *this;
  }

  ~DynamicArray() { free(_data); }

  void resize(size_t n, T v = 0) {
    if (_size < n) {
      _data = static_cast<T *>(realloc(_data, n * sizeof(T)));
      std::fill(_data + _size, _data + n, v);
    }

    _size = n;
  }

  T *data() { return _data; }
  const T *data() const { return _data; }

  T &operator[](size_t i) { return _data[i]; }
  const T &operator[](size_t i) const { return _data[i]; }

  size_t size() const { return _size; }
};