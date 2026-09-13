cmake_minimum_required(VERSION 3.16)
project(modelicacc)
include(ExternalProject)
include(FetchContent)

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
# 3rd party dependencies: Boost 1.81, sb-graph-legacy, sb-graph-dev, GTest
# ------------------------------------------------------------------------------

set(3RD_PARTY_DIR   ${CMAKE_SOURCE_DIR}/3rd-party)
set(SBG_LIB_DIR     ${3RD_PARTY_DIR}/sbg)
set(SBG_1_0_0_DIR   ${SBG_LIB_DIR}/sb-graph-1.0.0)
set(SBG_DEV_DIR     ${SBG_LIB_DIR}/sb-graph-dev)
set(BOOST_LIB_PATH  ${3RD_PARTY_DIR}/boost)
set(BOOST_1_81_DIR  ${BOOST_LIB_PATH}/boost-1.81.0)
set(GTEST_DIR       ${3RD_PARTY_DIR}/gtest)

# ------------------------------------------------------------------------------
# Boost 1.81: header library, it only extract the required contents
# ------------------------------------------------------------------------------

ExternalProject_Add(lib-boost
  PREFIX            ${BOOST_LIB_PATH}
  DOWNLOAD_COMMAND  ""
  SOURCE_DIR        ${BOOST_LIB_PATH}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND ${CMAKE_COMMAND} -E chdir ${BOOST_LIB_PATH} ${CMAKE_COMMAND} -E tar xf ${BOOST_LIB_PATH}/boost-1.81.0.tar.xz
  BUILD_ALWAYS      0
)

# ------------------------------------------------------------------------------
# GTest 1.12.1
# ------------------------------------------------------------------------------

ExternalProject_Add(lib-gtest
  PREFIX              ${GTEST_DIR}
  DOWNLOAD_COMMAND    ${CMAKE_COMMAND} -E chdir ${GTEST_DIR} ${CMAKE_COMMAND} -E tar xzf ${GTEST_DIR}/googletest-release-1.12.1.tar.gz
  SOURCE_DIR          ${GTEST_DIR}/googletest-release-1.12.1
  BINARY_DIR          ${GTEST_DIR}/build
  CMAKE_ARGS          -DCMAKE_INSTALL_PREFIX=${GTEST_DIR}/usr
  INSTALL_COMMAND     ${CMAKE_COMMAND} --build <BINARY_DIR> --target install
  BUILD_ALWAYS        0
  UPDATE_COMMAND      ""
  PATCH_COMMAND       ""
)

# ------------------------------------------------------------------------------
# sb-graph-legacy (sb-graph-1.0.0)
# ------------------------------------------------------------------------------

ExternalProject_Add(lib-sbg-legacy
  PREFIX              ${SBG_LIB_DIR}
  DOWNLOAD_COMMAND    ${CMAKE_COMMAND} -E chdir ${SBG_LIB_DIR} ${CMAKE_COMMAND} -E tar zxf sb-graph-1.0.0.tar.gz
  SOURCE_DIR          ${SBG_1_0_0_DIR}
  CONFIGURE_COMMAND   ${CMAKE_COMMAND} -E chdir "${SBG_1_0_0_DIR}" autoreconf -fi
  COMMAND             ${CMAKE_COMMAND} -E chdir "${SBG_1_0_0_DIR}" ./configure
  BUILD_COMMAND       make -C ${SBG_1_0_0_DIR}
  INSTALL_COMMAND     ${CMAKE_COMMAND} -E make_directory ${SBG_1_0_0_DIR}/usr
  COMMAND             make -C ${SBG_1_0_0_DIR} install prefix=${SBG_1_0_0_DIR}/usr
  BUILD_IN_SOURCE     1
  DEPENDS             lib-sbg-legacy
  UPDATE_COMMAND      ""
  PATCH_COMMAND       ""
)

# ------------------------------------------------------------------------------
# Configure options for the sb-graph-dev repo
# ------------------------------------------------------------------------------

set(SBG_BRANCH "sb-graph-dev" CACHE STRING "Branch of sb-graph-dev to checkout")
option(BUILD_SBG "Build the sb-graph-dev submodule" ON)
set(REPO_CHECKOUT "ssh" CACHE STRING "Checkout method for sb-graph-dev (ssh|https)")
set(CMAKE_DISABLE_TESTING ON CACHE BOOL "Disable testing" FORCE)

# ------------------------------------------------------------------------------
# Bring sb-graph-dev into ModelicaCC
# ------------------------------------------------------------------------------

ExternalProject_Add(update-sbg
  PREFIX              ${SBG_LIB_DIR}
  GIT_REPOSITORY      https://github.com/CIFASIS/sb-graph.git
  GIT_TAG             ${SBG_BRANCH}
  SOURCE_DIR          ${SBG_DEV_DIR}
  CONFIGURE_COMMAND   ""
  BUILD_COMMAND       ""
  INSTALL_COMMAND     ""
  UPDATE_DISCONNECTED 1
)

# ------------------------------------------------------------------------------
# Build sb-graph-dev 
# ------------------------------------------------------------------------------

if(BUILD_SBG)
  ExternalProject_Add(lib-sbg-dev
    PREFIX            ${SBG_LIB_DIR}
    DOWNLOAD_COMMAND  ""
    SOURCE_DIR        ${SBG_DEV_DIR}
    BINARY_DIR        ${SBG_DEV_DIR}/build
    INSTALL_COMMAND   ""
    DEPENDS           update-sbg lib-boost
    BUILD_ALWAYS      0
  )
else()
  add_custom_target(lib-sbg-dev DEPENDS lib-boost)
endif()

add_custom_target(lib-sbg DEPENDS lib-sbg-dev lib-sbg-legacy)

# ------------------------------------------------------------------------------
# Includes and links for sb-graph-legacy, sb-graph-dev and Boost 
# ------------------------------------------------------------------------------

add_library(sbg_legacy INTERFACE)
target_include_directories(sbg_legacy INTERFACE ${SBG_1_0_0_DIR}/usr/include)
target_link_directories(sbg_legacy INTERFACE ${SBG_1_0_0_DIR}/usr/lib)
target_link_libraries(sbg_legacy INTERFACE sbgraph)
add_dependencies(sbg_legacy lib-sbg-legacy)
add_library(SBG::legacy ALIAS sbg_legacy)

add_library(sbg_dev INTERFACE)
target_include_directories(sbg_dev INTERFACE ${SBG_DEV_DIR})
target_link_directories(sbg_dev INTERFACE
  ${SBG_DEV_DIR}/build
  ${SBG_DEV_DIR}/build/sbg
  ${SBG_DEV_DIR}/build/eval
  ${SBG_DEV_DIR}/build/parser
  ${SBG_DEV_DIR}/build/util
)
target_link_libraries(sbg_dev INTERFACE
  -Wl,--start-group
  sbgraph
  sbg-dev
  sbg-eval-lib
  sbg-parser-lib
  sbg-util-lib
  -Wl,--end-group
)
add_dependencies(sbg_dev lib-sbg-dev)
add_library(SBG::dev ALIAS sbg_dev)

add_library(boost_1_81 INTERFACE)
target_include_directories(boost_1_81 INTERFACE ${BOOST_1_81_DIR}/include)
add_dependencies(boost_1_81 lib-boost)
add_library(Boost::boost_1_81 ALIAS boost_1_81)

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
add_dependencies(modelica lib-sbg)

# ------------------------------------------------------------------------------
# Submodules of ModelicaCC
# ------------------------------------------------------------------------------

add_subdirectory(mmo)
add_subdirectory(antialias)
add_subdirectory(flatter)
add_subdirectory(causalize)

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
# ModelicaCC testing
# ------------------------------------------------------------------------------

add_custom_target(modelicacc_test
  COMMAND ${CMAKE_COMMAND} --build . --target flatter_test
  COMMAND ${CMAKE_COMMAND} --build . --target causalize_test
  DEPENDS lib-gtest
  COMMENT "Running test suite"
)

# ------------------------------------------------------------------------------
# Help for the user
# ------------------------------------------------------------------------------

add_custom_target(help_modelicacc
  COMMAND ${CMAKE_COMMAND} -E echo "cmake -DCMAKE_BUILD_TYPE=<Debug|Release> \
    -DSBG_BRANCH=<branch> -DBUILD_SBG=<ON|OFF> -DREPO_CHECKOUT=<ssh|https> .."
  COMMAND ${CMAKE_COMMAND} -E echo "Default values:"
  COMMAND ${CMAKE_COMMAND} -E echo "CMAKE_BUILD_TYPE=Release"
  COMMAND ${CMAKE_COMMAND} -E echo "SBG_BRANCH=sb-graph-dev"
  COMMAND ${CMAKE_COMMAND} -E echo "BUILD_SBG=ON"
  COMMAND ${CMAKE_COMMAND} -E echo "REPO_CHECKOUT=ssh"
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
