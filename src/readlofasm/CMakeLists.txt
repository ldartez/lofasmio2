cmake_minimum_required(VERSION 3.0)
project(readlofasm)
add_executable(readlofasm readlofasm.cpp ../lofasmfileheader.cpp)
  target_link_libraries(readlofasm lofasm z)
