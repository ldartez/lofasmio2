cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#### note #####
#### boost libraries & headers must be included manually if not in path
include_directories("include/")
add_subdirectory(src)
