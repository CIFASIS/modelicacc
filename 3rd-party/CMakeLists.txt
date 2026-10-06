include(FetchContent)

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
  find_package(benchmark 1.6 QUIET)

  if(NOT benchmark_FOUND)
    message(STATUS "Google Benchmark not found in the system, downloading with FetchContent")

    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
      google_benchmark
      GIT_REPOSITORY https://github.com/google/benchmark.git
      GIT_TAG        v1.6.1
      GIT_SHALLOW    TRUE
    )

    FetchContent_MakeAvailable(google_benchmark)
  endif()
endif()
