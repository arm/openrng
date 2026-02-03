/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <openrng.h>

#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#define LINE_BUF_SIZE 40

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <numbers file> <ndim> <nelem>\n";
    return 1;
  }

  const char *params_path = argv[1];
  // Anything up to 21201 with new-joe-kuo-6.21201
  const int ndim = std::stoi(argv[2]);
  const int nelem = std::stoi(argv[3]);

  std::ifstream params_file(params_path, std::ios::in);

  char buffer[LINE_BUF_SIZE];
  // Skip first line - this is just column headers
  params_file.getline(buffer, LINE_BUF_SIZE, '\n');

  std::vector<uint32_t> degree, poly;
  std::vector<std::vector<uint32_t>> init_dir;

  // Read degrees, polys and initial direction numbers into vectors
  uint32_t max_deg = 0;
  // Up to ndim - 1, as first dimension is not overridden
  for (int i = 0; i < ndim - 1; i++) {
    /* d: dimension index
       s: degree of polynomial
       a: polynomial.  */
    uint32_t d, s, a;
    params_file >> d >> s >> a;
    degree.push_back(s);
    // Post-process polynomial: Joe & Kuo do not encode the x^0 term or x^s
    // term. These terms are expected by VSL, so manually set them
    poly.push_back(1 | (a << 1) | (1 << s));

    max_deg = std::max(max_deg, s);

    std::vector<uint32_t> m(s);
    for (int j = 0; j < s; j++)
      params_file >> m[j];
    init_dir.emplace_back(std::move(m));
  }

  std::vector<uint32_t> params(1 +                  // dimen
                               2 +                  // 2 special flags
                               ndim - 1 +           // polys
                               1 +                  // maxdeg
                               (ndim - 1) * max_deg // init dir nums
  );

  params[0] = ndim;
  params[1] = VSL_USER_QRNG_INITIAL_VALUES;
  params[2] = VSL_USER_INIT_DIRECTION_NUMBERS | VSL_USER_PRIMITIVE_POLYMS;

  std::copy(poly.begin(), poly.end(), &params[3]);

  // Max degree when both poly and dir nums provided
  params[2 + ndim] = max_deg;

  // Initial direction nums - copy arrays across, leaving zero-padding so that
  // each row is max_deg elements long
  for (int d = 0; d < ndim - 1; d++)
    std::copy(init_dir[d].begin(), init_dir[d].end(),
              &params[3 + ndim + d * max_deg]);

  VSLStreamStatePtr stream;
  int err;

  err = vslNewStreamEx(&stream, VSL_BRNG_SOBOL, params.size(), params.data());
  assert(err == VSL_ERROR_OK);

  std::vector<float> out(nelem);
  err =
      vsRngUniform(VSL_RNG_METHOD_UNIFORM_STD, stream, nelem, out.data(), 0, 1);
  assert(err == VSL_ERROR_OK);

  err = vslDeleteStream(&stream);
  assert(err == VSL_ERROR_OK);

  for (int i = 0; i < nelem; i++) {
    std::cout << out[i];
    if (i % ndim == ndim - 1 || i == nelem - 1)
      std::cout << '\n';
    else
      std::cout << ",\t";
  }
}
