find_package(Doxygen REQUIRED)

# Doxygen requires latex for parsing math environments.
find_package(LATEX REQUIRED)

configure_file(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in Doxyfile @ONLY)

add_custom_target(html
    COMMAND ${DOXYGEN_EXECUTABLE}
    DEPENDS Doxyfile ${OPENRNG_ROOT}/include/
)
