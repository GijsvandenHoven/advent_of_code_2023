cmake_minimum_required(VERSION 3.25)
project(advent_of_code_2023)

set(CMAKE_CXX_STANDARD 20)
find_package(OpenMP REQUIRED)

add_executable(main main.cpp util/Day.cpp)
target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)