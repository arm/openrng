set(TRNG_GENERATORS
  vsl/x86_64/generator/trng/nondeterministic_trng.cpp
)

set_source_files_properties(${TRNG_GENERATORS} PROPERTIES COMPILE_OPTIONS -mrdrnd)

target_include_directories(openrng PRIVATE vsl/x86_64)

target_sources(openrng PRIVATE
  vsl/x86_64/cpu_features.cpp

  ${TRNG_GENERATORS}
)
