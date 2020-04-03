cmake_minimum_required(VERSION 3.10.2)
project(test)

set(CMAKE_CXX_STANDARD 11)

# Sources
set(SRC_LIST
    main.cpp
)

add_executable(stdio stdio.cpp)
add_executable(stdioe stdioe.cpp)
add_executable(pipe pipe.cpp)
add_executable(fork fork.cpp)
add_executable(dup dup.cpp)
add_executable(hello hello.cpp)
add_executable(exec exec.cpp)

