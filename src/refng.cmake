add_library(refng)

target_sources(refng PRIVATE
  refng/c_api.cpp
  ${VSL_GENERIC_SOURCES}
)

target_include_directories(refng
  PUBLIC
    refng/include
  PRIVATE
    refng
    generic
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<TARGET_PROPERTY:vsl,INTERFACE_INCLUDE_DIRECTORIES>
)

target_compile_options(refng PRIVATE -Wall -Wextra -Wpedantic -Werror)
# Define openrng to refng to ensure different namespaces for sources
# which are used for both libraries
target_compile_definitions(refng PRIVATE GIT_HASH=0x${GIT_HASH}ull
                                         openrng=refng)
