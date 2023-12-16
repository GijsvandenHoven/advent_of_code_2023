cmake_minimum_required(VERSION 3.25)
project(advent_of_code_2023)

set(CMAKE_CXX_STANDARD 20)
find_package(OpenMP REQUIRED)

if (OPENMP_FOUND)
    message("OpenMP FOUND")
endif()

add_executable(main main.cpp util/Day.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp") # This wasn't always necessary but now there's OpenMP linker errors if I do not do this.
target_compile_options(main PUBLIC -O3) # godbolt seems to indicate things like std::fill does not use AVX registers without O3 for GCC. Cringe!

target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)