# 'Google_test' is the subproject name
project(Google_tests)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# 'lib' is the folder with Google Test sources
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# adding the Google_Tests_run target
add_executable(Google_Tests_run tests.cpp accurate_frequency_analyzer.h utility_functions.h)

# linking Google_Tests_run with MapWithGetVeryFrequent_lib which will be tested
target_link_libraries(Google_Tests_run map_with_get_very_frequent_lib)

target_link_libraries(Google_Tests_run gtest gtest_main)