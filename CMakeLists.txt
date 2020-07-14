cmake_minimum_required(VERSION 3.12)
project(VkTest)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(MapWithGetVeryFrequent_run ${SOURCE_FILES})

include_directories(map_get_fresh_top_k_lib)
add_subdirectory(map_get_fresh_top_k_lib)

target_link_libraries(MapWithGetVeryFrequent_run map_get_fresh_top_k_lib)

add_subdirectory(google_tests)