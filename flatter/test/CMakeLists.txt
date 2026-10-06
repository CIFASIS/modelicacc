# ---------------------------------------------------------------------------
# Flatter test binary
# ---------------------------------------------------------------------------

set(FLATTER_TEST_SRC
  main.cpp
  models_test.cpp
)

add_executable(test-flatter ${FLATTER_TEST_SRC})

set_target_properties(
  test-flatter PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_compile_options(test-flatter PRIVATE -Wall)

# ---------------------------------------------------------------------------
# Dependencies: gtest, gmock
# ---------------------------------------------------------------------------

target_link_libraries(
  test-flatter
  PRIVATE
  GTest::gtest
  GTest::gmock
  PkgConfig::GiNaC
)

# ---------------------------------------------------------------------------
# CTest registration
# ---------------------------------------------------------------------------

add_test(
  NAME flatter
  COMMAND test-flatter
)

set_tests_properties(
  flatter PROPERTIES
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# ---------------------------------------------------------------------------
# Run flatter tests
# ---------------------------------------------------------------------------

set(FLATTER_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test_data)

add_custom_target(flatter_test
  COMMAND ${CMAKE_COMMAND} -E echo "Clean up test data dir."
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${FLATTER_TEST_DATA_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${FLATTER_TEST_DATA_DIR}
  COMMAND ${CMAKE_COMMAND} -E echo "Done"
  COMMAND ${CMAKE_COMMAND} -E echo "Running tests."
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "^flatter$"
  COMMAND ${CMAKE_COMMAND} -E echo "Done."
  DEPENDS test-flatter
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Building and running flatter tests"
)

# ---------------------------------------------------------------------------
# Clean flatter/test/
# ---------------------------------------------------------------------------

add_custom_target(clean-flatter-test
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/bin
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${FLATTER_TEST_DATA_DIR}
  COMMENT "Removing flatter/test bin/ and test_data/"
)
