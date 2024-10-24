set(VSL_GENERIC_SOURCES
  generic/generator/generator.cpp
  generic/generator/mcg31m1.cpp
  generic/generator/mcg59.cpp
  generic/generator/mrg32k3a.cpp
  generic/generator/mt19937.cpp
  generic/generator/nondeterministic.cpp
  generic/generator/philox4x3210.cpp
  generic/generator/r250.cpp
  generic/generator/sobol.cpp
  generic/generator/sfmt19937.cpp
  utils/mt_skip_ahead.cpp
)

if (NOT OPENRNG_TEST_TARGET)
  include(vsl.cmake)
endif ()

include(refng.cmake)
