cmake_minimum_required(VERSION 3.16)
project(modelicacc)
include(ExternalProject)
include(FetchContent)
include(GNUInstallDirs)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(
    CMAKE_INSTALL_PREFIX
    "$ENV{HOME}/.local/modelicacc"
    CACHE PATH
    "Installation prefix"
    FORCE
  )
endif()

option(MODELICACC_BUILD_TESTS "Build ModelicaCC tests" ON)
option(MODELICACC_BUILD_BENCHMARKS "Build ModelicaCC benchmarks" ON)

# ------------------------------------------------------------------------------
# Build type: Debug, Release
# ------------------------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Debug|Release)" FORCE)
endif()

# ------------------------------------------------------------------------------
# Global compilation flags
# ------------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wno-reorder)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-ggdb -fsanitize=address -O0)
  add_link_options(-fsanitize=address)
else()
  add_compile_options(-O3)
endif()

# ------------------------------------------------------------------------------
# ModelicaCC dependencies: sbgraph, ginac
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
# Output directories: bin/ and lib/
# ------------------------------------------------------------------------------

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

include_directories(${CMAKE_SOURCE_DIR})

# ------------------------------------------------------------------------------
# ModelicaCC submodules directories
# ------------------------------------------------------------------------------

set(ANTIALIAS_DIR "${CMAKE_CURRENT_LIST_DIR}/antialias")
set(AST_DIR "${CMAKE_CURRENT_LIST_DIR}/ast")
set(CAUSALIZE_DIR "${CMAKE_CURRENT_LIST_DIR}/causalize")
set(FLATTER_DIR "${CMAKE_CURRENT_LIST_DIR}/flatter")
set(MMO_DIR "${CMAKE_CURRENT_LIST_DIR}/mmo")
set(PARSER_DIR "${CMAKE_CURRENT_LIST_DIR}/parser")
set(TEST_DIR "${CMAKE_CURRENT_LIST_DIR}/test")
set(UTIL_DIR "${CMAKE_CURRENT_LIST_DIR}/util")


# ------------------------------------------------------------------------------
# Common files: ast/, parser/
# ------------------------------------------------------------------------------

set(COMMON_SRC
  ${AST_DIR}/expression.cpp
  ${AST_DIR}/class.cpp
  ${AST_DIR}/equation.cpp
  ${AST_DIR}/statement.cpp
  ${AST_DIR}/modification.cpp
  ${AST_DIR}/element.cpp
  ${PARSER_DIR}/ident.cpp
  ${PARSER_DIR}/expression.cpp
  ${PARSER_DIR}/modification.cpp
  ${PARSER_DIR}/equation.cpp
  ${PARSER_DIR}/statement.cpp
  ${PARSER_DIR}/class.cpp
  ${PARSER_DIR}/parser.cpp
)

# ------------------------------------------------------------------------------
# libmodelica.a
# ------------------------------------------------------------------------------

add_library(modelica STATIC ${COMMON_SRC})
set_target_properties(modelica PROPERTIES OUTPUT_NAME "modelica")
target_link_libraries(modelica PUBLIC SBG::sbgraph)

# ------------------------------------------------------------------------------
# ModelicaCC profiler
# ------------------------------------------------------------------------------

add_library(modelicacc-profiler STATIC
  ${UTIL_DIR}/profiler.cpp
)

target_include_directories(modelicacc-profiler
  PUBLIC
    ${CMAKE_SOURCE_DIR}
)

target_compile_features(modelicacc-profiler
  PUBLIC
    cxx_std_17
)

add_library(ModelicaCC::profiler ALIAS modelicacc-profiler)

# ------------------------------------------------------------------------------
# Testing dependencies: GTest, Google Benchmark
# ------------------------------------------------------------------------------

if(MODELICACC_BUILD_TESTS OR MODELICACC_BUILD_BENCHMARKS)
  add_subdirectory(3rd-party)
  enable_testing()
endif()

# ------------------------------------------------------------------------------
# Submodules of ModelicaCC
# ------------------------------------------------------------------------------

add_subdirectory(mmo)
add_subdirectory(antialias)
add_subdirectory(flatter)
add_subdirectory(causalize)

# ------------------------------------------------------------------------------
# ModelicaCC testing
# ------------------------------------------------------------------------------

if(MODELICACC_BUILD_TESTS)
  add_custom_target(modelicacc_test
    #COMMAND ${CMAKE_COMMAND} --build . --target flatter_test
    COMMAND ${CMAKE_COMMAND} --build . --target causalize_test
    DEPENDS GTest::gtest  GTest::gmock GTest::gtest_main
    COMMENT "Running test suite"
  )
endif()

# ------------------------------------------------------------------------------
# Install: cmake --install <build-dir> [--prefix <dir>]
# ------------------------------------------------------------------------------

install(
  TARGETS mmo antialias flatter causalize
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  TARGETS modelica modelicacc-profiler
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# ------------------------------------------------------------------------------
# Generate documentation 
# ------------------------------------------------------------------------------

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
  add_custom_target(modelicacc_doc
    COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating documentation with Doxygen"
  )
endif()

# ------------------------------------------------------------------------------
# Help for the user
# ------------------------------------------------------------------------------

add_custom_target(help_modelicacc
  COMMAND ${CMAKE_COMMAND} -E echo "Default values:"
  COMMAND ${CMAKE_COMMAND} -E echo "CMAKE_BUILD_TYPE=Release"
  COMMAND ${CMAKE_COMMAND} -E echo "CMAKE_PREFIX_PATH=$HOME/.local/sbgraph"
  COMMAND ${CMAKE_COMMAND} -E echo "SBG_REF=sb-graph-dev"
  COMMAND ${CMAKE_COMMAND} -E echo "CMAKE_INSTALL_PREFIX=$HOME/.local/modelicacc"
  COMMAND ${CMAKE_COMMAND} -E echo "MODELICACC_BUILD_TESTS=ON"
  COMMAND ${CMAKE_COMMAND} -E echo "MODELICACC_BUILD_BENCHMARKS=ON"
)

# ------------------------------------------------------------------------------
# clean-all: deletes bin/, lib/ and doc/
# ------------------------------------------------------------------------------

add_custom_target(clean-all
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/bin
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/lib
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/doc
  COMMENT "Removing bin/, lib/ and doc/"
)
