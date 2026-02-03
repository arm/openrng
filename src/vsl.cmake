add_library(openrng)
if(OPENRNG_LIB_SUFFIX)
  set_target_properties(openrng PROPERTIES OUTPUT_NAME "openrng_${OPENRNG_LIB_SUFFIX}")
endif()

# Passing the library version to the target
target_compile_definitions(openrng PUBLIC OPENRNG_MAJOR=${OPENRNG_MAJOR})
target_compile_definitions(openrng PUBLIC OPENRNG_MINOR=${OPENRNG_MINOR})
target_compile_definitions(openrng PUBLIC OPENRNG_PATCH=${OPENRNG_PATCH})

#
# Set VERSION and SOVERSION. This can help system administrators with upgrading
# or downgrading OpenRNG.
#
# In the specific case of Linux, when compiling an executable, a user will
# typically target libopenrng.so, no suffix. Assuming VERSION=24.04 and
# SOVERSION=1, when running an executable, it will look for libopenrng.so.1,
# with the SOVERSION suffix. The actual executable is libopenrng.24.04, with the
# VERSION suffix. A system administrator can install multiple versions of
# OpenRNG, and then configure the symbolic links to pick the desired version at
# either compile or runtime. The default installation on linux looks like.
#
#    $ ls install/lib -l
#   lrwxrwxrwx libopenrng.so -> libopenrng.so.1
#   lrwxrwxrwx libopenrng.so.1 -> libopenrng.so.24.04
#   -rw-r--r-- libopenrng.so.24.04
#
# And the SOVERSION is embedded in the library's header.
#
#    $ readelf -d install/lib/libopenrng.so.24.04 | grep SONAME
#   0x000000000000000e (SONAME)             Library soname: [libopenrng.so.1]
#
# VERSION: This needs to be bumped every release.
#
# SOVERSION: If the ABI has changed since the previous release, this number
# needs to be bumped.
#
set_target_properties(openrng PROPERTIES VERSION ${OPENRNG_VERSION} SOVERSION 2)

# CMAKE_SYSTEM_PROCESSOR is the target processor, if not set, it will default to
# the host platform. If you want to change CMAKE_SYSTEM_PROCESSOR, you will also
# need to set CMAKE_SYSTEM_NAME. This can be done on the command line with
# -DCMAKE_SYSTEM_NAME=... -DCMAKE_SYSTEM_PROCESSOR=..., but typically it is done
# via a toolchain file.

if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm64|aarch64|ARM64")
  include(aarch64.cmake)
elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64|AMD64")
  include(x86_64.cmake)
else()
  message(FATAL_ERROR "Unsupported target processor: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

target_sources(openrng PRIVATE
  vsl/c_api.cpp
  ${VSL_GENERIC_SOURCES}
)

# libc and libm don't exist on Windows. Let the linker decide for itself.
if(NOT WIN32)
  target_link_libraries(openrng c m)
endif()

target_include_directories(openrng
  PUBLIC
    ../include
  PRIVATE
    generic
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(openrng PRIVATE -fno-exceptions -fno-rtti -Wall -Wextra -Wpedantic -Wshadow -Werror -fno-math-errno)

target_compile_definitions(openrng PRIVATE GIT_HASH=0x${GIT_HASH}ull)

# Installation rules
include(GNUInstallDirs)
set(OPENRNG_PUBLIC_HEADERS
  ../include/openrng.h
)
set_target_properties(openrng PROPERTIES PUBLIC_HEADER "${OPENRNG_PUBLIC_HEADERS}")
install(TARGETS openrng)

#
# To improve portability, we avoid depending on the C++ runtime. For a shared
# library we pass -nodefaultlibs to ensure the linker doesn't introduce a
# dependency. We then test linking continues to work with
# test_linking_with_c_compiler.
#
if(${BUILD_SHARED_LIBS})
  target_link_options(openrng PRIVATE "-nodefaultlibs")
endif()

# Copy openrng.h.json file to location.
set(interface_json openrng.h.json)
configure_file(${OPENRNG_ROOT}/tools/create_api/openrng.h.json ${CMAKE_CURRENT_BINARY_DIR}/openrng.h.json )


# Create a C source file that makes a call to every routine in the interface.
set(link_src_file linking_with_c.c)
add_custom_command(
  OUTPUT ${link_src_file}
  COMMAND ${OPENRNG_ROOT}/tools/create_api/create_link_test.py ${link_src_file}
  DEPENDS ${OPENRNG_ROOT}/tools/create_api/create_link_test.py ${CMAKE_CURRENT_BINARY_DIR}/${interface_json})

# Test linking a C executable by disabling implicit c++ libraries and forcing the linker language.
add_executable(test_linking_with_c_compiler ${link_src_file})
target_link_libraries(test_linking_with_c_compiler openrng)
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")
set_target_properties(test_linking_with_c_compiler PROPERTIES LINKER_LANGUAGE C)

# Test library version
add_executable(library_version ${OPENRNG_ROOT}/tools/check_library_version/check_library.c)
target_link_libraries(library_version openrng)
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")
set_target_properties(library_version PROPERTIES LINKER_LANGUAGE C)

# Enable optimization records and inspect assembly (only in later versions of GCC/clang).
# Outputs reports to the same directory as the obj files.
if(OPENRNG_ENABLE_OPT_RECORDS)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-foptimization-record-passes=vector|inline HAS_OPT_REPORT_FILTER)
  check_cxx_compiler_flag(-fsave-optimization-record HAS_OPT_REPORT_SAVE)
  check_cxx_compiler_flag(--save-temps=obj HAS_ASSEMBLY_SAVE)

  if(HAS_OPT_REPORT_FILTER)
    target_compile_options(openrng PRIVATE -foptimization-record-passes=vector|inline)
  endif(HAS_OPT_REPORT_FILTER)

  if(HAS_OPT_REPORT_SAVE)
    target_compile_options(openrng PRIVATE -fsave-optimization-record)
  endif(HAS_OPT_REPORT_SAVE)

  if(HAS_ASSEMBLY_SAVE)
    target_compile_options(openrng PRIVATE --save-temps=obj -Wno-gnu-line-marker)
  endif(HAS_ASSEMBLY_SAVE)
endif(OPENRNG_ENABLE_OPT_RECORDS)

# Configure test target and list all incompatible generators that are to be excluded
# from bitwise compatible unit tests when testing OpenRNG against other VSL libraries,
# such as ArmPL. The variable is a semicolon separated string, e.g. "MT2203;NIEDERR",
# The code will exclude all generators of the form VSL_BRNG_{gen_name}, where gen_name
# is an item of the list
add_library(vsl ALIAS openrng)
set(OPENRNG_TEST_SPEC "~[no_openrng]" PARENT_SCOPE)
set(OPENRNG_INCOMPATIBLE_GENERATORS "" PARENT_SCOPE)
