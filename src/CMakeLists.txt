set(GENERATE_NIEDERR_POLYNOMIALS ${OPENRNG_ROOT}/tools/niederr_irreducible_polynomials/niederr_irreducible_polynomials.py)
set(NIEDERR_POLYNOMIALS ${CMAKE_CURRENT_BINARY_DIR}/niederr_default_initialization.cpp)
add_custom_command(
  OUTPUT ${NIEDERR_POLYNOMIALS}
  COMMAND ${GENERATE_NIEDERR_POLYNOMIALS} 318 --cpp > ${NIEDERR_POLYNOMIALS}
  DEPENDS ${GENERATE_NIEDERR_POLYNOMIALS}
)

set(VSL_GENERIC_SOURCES
  generic/generator/ars5.cpp
  generic/generator/generator.cpp
  generic/generator/mcg31m1.cpp
  generic/generator/mcg59.cpp
  generic/generator/mrg32k3a.cpp
  generic/generator/mt19937.cpp
  generic/generator/mt2203.cpp
  generic/generator/mt2203_params.cpp
  generic/generator/niederr.cpp
  generic/generator/nondeterministic.cpp
  generic/generator/philox4x3210.cpp
  generic/generator/qrng_base_generator.cpp
  generic/generator/r250.cpp
  generic/generator/sfmt19937.cpp
  generic/generator/sobol.cpp

  ${NIEDERR_POLYNOMIALS}
)

if (NOT OPENRNG_TEST_TARGET)
  include(vsl.cmake)
endif ()

include(refng.cmake)
