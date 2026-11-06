find_package(benchmark REQUIRED)
find_package(Boost REQUIRED)

add_executable(modelicacc-sbg-benchmark main.cpp)

target_include_directories(modelicacc-sbg-benchmark
  PRIVATE ${PROJECT_SOURCE_DIR})

target_sources(modelicacc-sbg-benchmark
  PRIVATE
  main.cpp
  sbg_bm.cpp
  scalar/build_graphs.cpp
  scalar/mfvs.cpp
  scalar/mfvs_bm.cpp
  scalar/scalar_utils.cpp
  scalar/sort_bm.cpp)

set_target_properties(modelicacc-sbg-benchmark PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

target_link_libraries(
  modelicacc-sbg-benchmark
  PRIVATE
  benchmark::benchmark
  Boost::boost
  causalize-lib
  modelica
  ModelicaCC::profiler
  SBG::sbgraph
)

add_custom_target(run-benchmark
      COMMAND echo "Running SBG/scalar Benchmark."
      COMMAND cd ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} && ./modelicacc-sbg-benchmark
)

add_subdirectory(scalar)
