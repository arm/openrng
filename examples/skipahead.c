/*
 * SPDX-FileCopyrightText: <text>Copyright 2024-2026 Arm Limited and/or its
 * affiliates <open-source-office@arm.com></text>
 *
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <openrng.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void assert_ok(int err, const char *message) {
  if (err != VSL_ERROR_OK) {
    printf("Error: %s\n", message);
    exit(EXIT_FAILURE);
  }
}

int do_sample(VSLStreamStatePtr stream, float *buf, int n) {
  return vsRngGaussian(VSL_RNG_METHOD_GAUSSIAN_ICDF, stream, n, buf, 0, 1);
}

int main() {

  /* Generate N random numbers. For large N it may be more efficient to use the
     'block-splitting' method, demonstrated here.  */

  const int N = 1000000;
  const int brng = VSL_BRNG_MCG31;
  const int seed = 1;

  /* Needs to be a factor of N.  */
  const int n_threads = 16;

  int err;

  /* First do a serial sample of N for comparison.  */
  float serial_buffer[N];
  VSLStreamStatePtr serial_stream;
  err = vslNewStream(&serial_stream, brng, seed);
  assert_ok(err, "vslNewStreamFailed");

  clock_t before_serial = clock();
  err = do_sample(serial_stream, serial_buffer, N);
  clock_t after_serial = clock();
  assert_ok(err, "serial sample failed");
  printf("Serial run took %f seconds\n",
         (double)(after_serial - before_serial) / CLOCKS_PER_SEC);

  /* Now do the same sample in parallel with n_threads streams, each generating
     a segment of the array independently.  */
  float parallel_buffer[N];
  VSLStreamStatePtr parallel_streams[n_threads];

  /* Initialise the first stream normally, then initialise n_threads subsequent
     streams by copying and skipping the previous one.  */
  err = vslNewStream(&parallel_streams[0], brng, seed);
  assert_ok(err, "Could not initialise parallel stream 0");
  for (int i = 1; i < n_threads; i++) {
    err = vslCopyStream(&parallel_streams[i], parallel_streams[i - 1]);
    assert_ok(err, "Could not copy stream");
    err = vslSkipAheadStream(parallel_streams[i], N / n_threads);
    assert_ok(err, "Could not skip");
  }

  /* Run each sample in parallel.  */
  clock_t before_parallel = clock();
#pragma omp parallel for
  for (int i = 0; i < n_threads; i++) {
    const int p_err =
        do_sample(parallel_streams[i], parallel_buffer + i * (N / n_threads),
                  N / n_threads);
    assert_ok(p_err, "Parallel sample failed");
  }
  clock_t after_parallel = clock();
  printf("Parallel run took %f seconds\n",
         (double)(after_parallel - before_parallel) / CLOCKS_PER_SEC);

  /* Exit with failure if any elements do not match.  */
  for (int i = 0; i < N; i++) {
    if (serial_buffer[i] != parallel_buffer[i]) {
      printf("serial_buffer[%d]   = %a\n", i, serial_buffer[i]);
      printf("parallel_buffer[%d] = %a\n", i, parallel_buffer[i]);
      exit(EXIT_FAILURE);
    }
  }

  puts("All values matched!");

  /* Deallocate all resources used.  */
  err = vslDeleteStream(&serial_stream);
  assert_ok(err, "vslDeleteStream failed");
  for (int i = 0; i < n_threads; i++) {
    err = vslDeleteStream(&parallel_streams[i]);
    assert_ok(err, "vslDeleteStream failed");
  }
}
