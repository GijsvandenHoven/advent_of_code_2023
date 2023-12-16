cmake_minimum_required(VERSION 3.25)
project(advent_of_code_2023)

set(CMAKE_CXX_STANDARD 20)
find_package(OpenMP REQUIRED)

add_executable(main main.cpp util/Day.cpp)
target_compile_options(main PUBLIC -O3) # godbolt seems to indicate things like std::fill does not use AVX registers without O3 for GCC. Cringe!
target_link_libraries(main PRIVATE -fopenmp)