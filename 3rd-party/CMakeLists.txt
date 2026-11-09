include(FetchContent)

# ------------------------------------------------------------------------------
# SBG library
# ------------------------------------------------------------------------------

# Keep this before include_directories() and the output directories: they
# would leak into sb-graph when it is fetched (e.g. ModelicaCC's util/ and
# parser/ headers shadowing sb-graph's ones).

set(CMAKE_PREFIX_PATH
  "$ENV{HOME}/.local/sbgraph"
  CACHE PATH
  "Paths where CMake looks for installed packages (e.g. sb-graph)"
)
set(SBG_REF "sb-graph-dev" CACHE STRING
  "Branch, tag or commit of sb-graph to fetch when SBGraph is not installed")

find_package(SBGraph CONFIG QUIET)

if(SBGraph_FOUND)
  message(STATUS "Using installed SBGraph: ${SBGraph_DIR}")
  set_target_properties(SBG::sbgraph PROPERTIES IMPORTED_GLOBAL TRUE)
else()
  message(STATUS "SBGraph not found, downloading sb-graph (${SBG_REF}) with FetchContent")

  # Only the library is needed, skip sb-graph tests, its 3rd-party deps and
  # docs (same settings as .github/workflows/c-cpp.yml).
  set(SBG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(SBG_BUILD_DOCS OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(
    sbgraph
    GIT_REPOSITORY https://github.com/CIFASIS/sb-graph.git
    GIT_TAG        ${SBG_REF}
    GIT_SHALLOW    TRUE
  )

  FetchContent_MakeAvailable(sbgraph)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GiNaC REQUIRED IMPORTED_TARGET ginac)

# ------------------------------------------------------------------------------
# Boost
# ------------------------------------------------------------------------------

find_package(Boost 1.81 QUIET)

if(Boost_FOUND)
  foreach(_boost_tgt Boost::boost Boost::headers)
    if(TARGET ${_boost_tgt})
      get_target_property(_aliased ${_boost_tgt} ALIASED_TARGET)
      get_target_property(_is_global ${_boost_tgt} IMPORTED_GLOBAL)
      if(NOT _aliased AND NOT _is_global)
        set_target_properties(${_boost_tgt} PROPERTIES IMPORTED_GLOBAL TRUE)
      endif()
    endif()
  endforeach()
else()
  message(STATUS "Boost not found in the system, downloading with FetchContent")

  string(REPLACE "." "_" _boost_underscored ${MODELICACC_BOOST_FETCH_VERSION})

  FetchContent_Declare(
    boost
    URL https://archives.boost.io/release/${MODELICACC_BOOST_FETCH_VERSION}/source/boost_${_boost_underscored}.tar.gz
    SOURCE_SUBDIR _no_cmake_
  )
  FetchContent_MakeAvailable(boost)

  add_library(modelicacc_boost_headers INTERFACE)
  target_include_directories(modelicacc_boost_headers SYSTEM INTERFACE ${boost_SOURCE_DIR})

  add_library(Boost::boost   ALIAS modelicacc_boost_headers)
  add_library(Boost::headers ALIAS modelicacc_boost_headers)
endif()

# ------------------------------------------------------------------------------
# GoogleTest / GoogleMock
# ------------------------------------------------------------------------------

if(MODELICACC_BUILD_TESTS)
  find_package(GTest 1.12 QUIET)

  if(NOT GTest_FOUND)
    message(STATUS "GTest not found in the system, downloading with FetchContent")

    # Evita instalar GTest/GMock cuando se instale ModelicaCC.
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    # En Windows evita que GTest reemplace la CRT del proyecto padre.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG        release-1.12.1
      GIT_SHALLOW    TRUE
    )

    FetchContent_MakeAvailable(googletest)
  endif()
endif()

# ------------------------------------------------------------------------------
# Google Benchmark
# ------------------------------------------------------------------------------

if(MODELICACC_BUILD_BENCHMARKS)
  find_package(benchmark 1.8 QUIET)

  if(NOT benchmark_FOUND)
    message(STATUS "Google Benchmark not found in the system, downloading with FetchContent")

    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
      google_benchmark
      GIT_REPOSITORY https://github.com/google/benchmark.git
      GIT_TAG        v1.8.0
      GIT_SHALLOW    TRUE
    )

    FetchContent_MakeAvailable(google_benchmark)
  else()
    set_target_properties(benchmark::benchmark PROPERTIES IMPORTED_GLOBAL TRUE)
  endif()
endif()
