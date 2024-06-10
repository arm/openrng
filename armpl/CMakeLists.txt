set(ARMPL_ROOT "$ENV{ARMPL_DIR}" CACHE STRING "Root of ArmPL install")

if ("${ARMPL_ROOT}" STREQUAL "")
  message(FATAL_ERROR "Cannot find ArmPL root - please set ARMPL_ROOT")
endif()

add_library(armpl SHARED IMPORTED GLOBAL)
if ("${OPENRNG_INTERFACE}" STREQUAL "ilp64")
  set_property(TARGET armpl PROPERTY IMPORTED_LOCATION
               ${ARMPL_ROOT}/lib/libarmpl_ilp64.so)
else ()
  set_property(TARGET armpl PROPERTY IMPORTED_LOCATION
               ${ARMPL_ROOT}/lib/libarmpl_lp64.so)
endif ()

set_property(TARGET armpl PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             ${ARMPL_ROOT}/include)

# Configure test target
add_library(vsl ALIAS armpl)
set(OPENRNG_TEST_SPEC "~[no_openrng]" PARENT_SCOPE)
