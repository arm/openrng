/*
 * SPDX-FileCopyrightText: <text>Copyright 2024 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include "bench.hpp"
#include "defines.hpp"

#include <openrng.h>

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "usage: " << argv[0] << " brng niters nskip" << std::endl;
    return EXIT_FAILURE;
  }
  const auto brng = openrng::bench::get_define(argv[1]);
  const auto niters = std::stoi(argv[2]);
  const auto nskip = std::stoull(argv[3], nullptr, 0);
  const auto result = openrng::bench::skipahead(niters, brng, nskip);
  std::cout << result << std::endl;
  return EXIT_SUCCESS;
}
