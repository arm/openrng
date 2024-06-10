set(GENERATE_SOBOL_TABLE ${OPENRNG_ROOT}/tools/generate_direction_nums/generate_direction_nums.py)
set(SOBOL_TABLE_AUTOGEN_FILE ${CMAKE_CURRENT_BINARY_DIR}/sobol_default_direction_numbers.cpp)
add_custom_command(
  OUTPUT ${SOBOL_TABLE_AUTOGEN_FILE}
  COMMAND ${GENERATE_SOBOL_TABLE} > ${SOBOL_TABLE_AUTOGEN_FILE}
  DEPENDS ${GENERATE_SOBOL_TABLE}
)

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

  ${SOBOL_TABLE_AUTOGEN_FILE}
)

if (NOT OPENRNG_TEST_TARGET)
  include(vsl.cmake)
endif ()

include(refng.cmake)
