cmake_minimum_required(VERSION 3.15)
project(rng VERSION 24.04 LANGUAGES C CXX)
include(CTest)

# Set build type to Release if not specified.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type specified. Defaulting to Release.")
  set(CMAKE_BUILD_TYPE RELEASE CACHE STRING "Choose the build type." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "RelWithDebInfo" "MinSizeRel")
endif()


# Set the standard to C++20 globally if it's supported, otherwise, print an
# error message.
file(
  WRITE ${CMAKE_CURRENT_BINARY_DIR}/concepts.cpp
  "#include <concepts>\nint main() {}"
)
try_compile(
  COMPILER_SUPPORTS_CXX20 ${CMAKE_CURRENT_BINARY_DIR} SOURCES
  ${CMAKE_CURRENT_BINARY_DIR}/concepts.cpp
  CXX_STANDARD 20
)
if (NOT COMPILER_SUPPORTS_CXX20)
  message(FATAL_ERROR "C++20 support is required. Configure CMake to use a C++20 compiler.")
endif()
set(CMAKE_CXX_STANDARD 20)

# Alternative to CMAKE_SOURCE_DIR. You can't include a CMAKE project that uses
# CMAKE_SOURCE_DIR
set(OPENRNG_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Enable position-independent code globally.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(OPENRNG_ENABLE_ASAN "Enable -fsanitize=address everywhere" OFF)
if(OPENRNG_ENABLE_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

# Create GIT_HASH variable of exactly 16 characters. Note: this only gets run on
# configure.
execute_process(
    COMMAND git log -n 1 --format=%h --abbrev=16
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Every time a generator is serialised, it has the GIT_HASH baked into it. This
# prevents OpenRNG from loading in a potentilly incompatible generator from a
# different build. This feature is only enabled if building from a git checkout.
if (NOT GIT_HASH)
  message(WARNING "Building outside Git, unique versioning disabled")
  set(GIT_HASH 0)
endif()

# Disable GLIBCXX assertions to avoid libstdc++ dependency
add_compile_definitions(_GLIBCXX_NO_ASSERTIONS)

if (WIN32)
  # Disable _CRT_SECURE and _CRT_NONSTDC for portability. We want to continue
  # using fopen, getpid, etc. on WIN32.
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
endif()

set(OPENRNG_INTERFACE "lp64" CACHE STRING "lp64 (32-bit int) or ilp64 (64-bit int) interface")
if(NOT ("${OPENRNG_INTERFACE}" STREQUAL "lp64" OR "${OPENRNG_INTERFACE}" STREQUAL "ilp64"))
  message(FATAL_ERROR "OPENRNG_INTERFACE must be either lp64 or ilp64")
endif()
set(OPENRNG_LIB_SUFFIX CACHE STRING "Suffix for the built library, for example \"ilp64\" for 64-bit builds")

set(OPENRNG_TEST_TARGET "" CACHE STRING "Alternative directory containing a CMakeLists.txt that defines vsl")
if(OPENRNG_TEST_TARGET)
  add_subdirectory(${OPENRNG_TEST_TARGET} vsl)
  message(STATUS "Testing ${OPENRNG_TEST_TARGET_NAME} against refng")
else()
  message(STATUS "Testing openrng against refng")
endif()

if("${OPENRNG_INTERFACE}" STREQUAL "ilp64")
  add_compile_definitions("OPENRNG_ILP64")
endif()

add_subdirectory(src)

# BUILD_TESTING is a default option enabled by the CTest module.
if(BUILD_TESTING)
    add_subdirectory(test)
endif()

option(BUILD_BENCH "Build benchmarks" ON)
IF(BUILD_BENCH)
    add_subdirectory(bench)
endif()

option(BUILD_DOCS "Build documentation" OFF)
IF(BUILD_DOCS)
    add_subdirectory(docs)
endif()
