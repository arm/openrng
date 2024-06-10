add_library(openrng)
if(OPENRNG_LIB_SUFFIX)
  set_target_properties(openrng PROPERTIES OUTPUT_NAME "openrng_${OPENRNG_LIB_SUFFIX}")
endif()

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

# Create a C source file that makes a call to every routine in the interface.
set(link_src_file linking_with_c.c)
add_custom_command(
  OUTPUT ${link_src_file}
  COMMAND ${OPENRNG_ROOT}/tools/create_api/create_link_test.py ${link_src_file}
  DEPENDS ${OPENRNG_ROOT}/tools/create_api/create_link_test.py)

# Test linking a C executable by disabling implicit c++ libraries and forcing the linker language.
add_executable(test_linking_with_c_compiler ${link_src_file})
target_link_libraries(test_linking_with_c_compiler openrng)
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")
set_target_properties(test_linking_with_c_compiler PROPERTIES LINKER_LANGUAGE C)

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

# Configure test target
add_library(vsl ALIAS openrng)
set(OPENRNG_TEST_SPEC "~[no_openrng]" PARENT_SCOPE)
