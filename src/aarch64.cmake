set(TRNG_GENERATORS
  vsl/aarch64/generator/trng/nondeterministic_trng.cpp
)

set_source_files_properties(${TRNG_GENERATORS} PROPERTIES COMPILE_OPTIONS -march=armv8.5-a+rng)

set(NEON_GENERATORS
  vsl/aarch64/generator/neon/mcg31m1_neon.cpp
  vsl/aarch64/generator/neon/mcg59_neon.cpp
  vsl/aarch64/generator/neon/mrg32k3a_neon.cpp
  vsl/aarch64/generator/neon/mt19937_neon.cpp
  vsl/aarch64/generator/neon/philox4x3210_neon.cpp
  vsl/aarch64/generator/neon/sfmt19937_neon.cpp
)

set(SHA3_GENERATORS
  vsl/aarch64/generator/sha3/mt19937_sha3.cpp
  vsl/aarch64/generator/sha3/philox4x3210_sha3.cpp
  vsl/aarch64/generator/sha3/sfmt19937_sha3.cpp
)

set_source_files_properties(${SHA3_GENERATORS} PROPERTIES COMPILE_OPTIONS -march=armv8.2-a+sha3)

# SVE support on Windows & macOS is poor, so skip building.
if(NOT (WIN32 OR APPLE))
  set(SVE_DISTRIBUTION_KERNELS
    vsl/aarch64/distributions/kernels/sve/box_muller_2.cpp
    vsl/aarch64/distributions/kernels/sve/exponential.cpp
    vsl/aarch64/distributions/kernels/sve/gaussian_icdf.cpp
    vsl/aarch64/distributions/kernels/sve/geometric.cpp
    vsl/aarch64/distributions/kernels/sve/rayleigh.cpp
  )
endif()
set_source_files_properties(${SVE_DISTRIBUTION_KERNELS} PROPERTIES COMPILE_OPTIONS -march=armv8-a+sve)

target_include_directories(openrng PRIVATE vsl/aarch64)

target_sources(openrng PRIVATE
  vsl/aarch64/cpu_features.cpp

  ${SVE_DISTRIBUTION_KERNELS}

  ${NEON_GENERATORS}
  ${TRNG_GENERATORS}
  ${SHA3_GENERATORS}
)
