cmake_minimum_required(VERSION 3.20)
project(unique_ptr)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(unique_ptr
    src/main.cpp
)
target_include_directories(unique_ptr PRIVATE include)
