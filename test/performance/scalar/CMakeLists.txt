add_executable(scalar-benchmark main.cpp)

target_include_directories(scalar-benchmark
  PRIVATE ${PROJECT_SOURCE_DIR})

target_sources(scalar-benchmark
  PRIVATE
  build_graphs.cpp
  main.cpp
  mfvs.cpp
  mfvs_bm.cpp
  scalar_utils.cpp
  sort_bm.cpp)

set_target_properties(scalar-benchmark PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

target_link_libraries(
  scalar-benchmark
  PRIVATE
  benchmark::benchmark
  Boost::boost
)
