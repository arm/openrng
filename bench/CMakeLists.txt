function(add_bench bench_src)
  get_filename_component(bench_name ${bench_src} NAME_WE)
  add_executable(bench_${bench_name} ${bench_src})
  target_include_directories(bench_${bench_name} PRIVATE include)
  target_link_libraries(bench_${bench_name} vsl)
endfunction()

set(DIST_JSON ${CMAKE_CURRENT_SOURCE_DIR}/scripts/distributions.json)
set(GENERATE_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_bench.py)
set(GENERATE_DIR ${CMAKE_CURRENT_BINARY_DIR}/src)

# Re-configure if benchmark generation script or data changes
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${DIST_JSON} ${GENERATE_SCRIPT})

# Invoke benchmark generation script at configure-time
file(MAKE_DIRECTORY ${GENERATE_DIR})
execute_process(
  COMMAND ${Python3_EXECUTABLE} ${GENERATE_SCRIPT} ${GENERATE_DIR}
  INPUT_FILE ${DIST_JSON}
  RESULT_VARIABLE generate_status
  ERROR_VARIABLE generate_error
)

if (generate_status AND NOT generate_status EQUAL 0)
  message(FATAL_ERROR "Failed to generate benchmarks:\n${generate_error}")
endif()

# Discover any generated benchmarks
file(GLOB benches CONFIGURE_DEPENDS ${GENERATE_DIR}/*.cpp)
foreach(bench ${benches})
  add_bench(${bench})
endforeach()

add_bench(src/skipahead.cpp)
