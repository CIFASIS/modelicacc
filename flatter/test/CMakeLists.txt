# ---------------------------------------------------------------------------
# Flatter test binary
# ---------------------------------------------------------------------------

set(FLATTER_TEST_SRC
  main.cpp
  models_test.cpp
)

add_executable(test-flatter ${FLATTER_TEST_SRC})

# ---------------------------------------------------------------------------
# Dependencies: gtest, gmock
# ---------------------------------------------------------------------------

set(FLATTER_TEST_GTEST_ROOT ${CMAKE_SOURCE_DIR}/3rd-party/gtest/usr)

target_compile_options(test-flatter PRIVATE -Wall)
set_target_properties(test-flatter PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

target_include_directories(test-flatter PRIVATE ${FLATTER_TEST_GTEST_ROOT}/include)
target_link_directories(test-flatter PRIVATE ${FLATTER_TEST_GTEST_ROOT}/lib)

target_link_libraries(test-flatter PRIVATE
  gtest
  gmock
  pthread
  ginac
  cln
  gmp
)

add_dependencies(test-flatter lib-gtest)

# ---------------------------------------------------------------------------
# Create bin/ and test_data/ in current directory
# ---------------------------------------------------------------------------

set_target_properties(test-flatter PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)

set(FLATTER_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test_data)

# ---------------------------------------------------------------------------
# Run flatter tests
# ---------------------------------------------------------------------------

add_custom_target(flatter_test
  COMMAND ${CMAKE_COMMAND} -E echo "Clean up test data dir."
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${FLATTER_TEST_DATA_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${FLATTER_TEST_DATA_DIR}
  COMMAND ${CMAKE_COMMAND} -E echo "Done"
  COMMAND ${CMAKE_COMMAND} -E echo "Running tests."
  COMMAND $<TARGET_FILE:test-flatter> || true
  COMMAND ${CMAKE_COMMAND} -E echo "Done."
  DEPENDS test-flatter
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
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
