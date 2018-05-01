cmake_minimum_required(VERSION 3.0)
project(readlofasm)
add_executable(lofasm2bbx lofasm2bbx.cpp ../lofasmfileheader.cpp)
target_link_libraries(lofasm2bbx lofasm z)
