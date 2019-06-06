cmake_minimum_required(VERSION 3.0)

# add static libraries
add_library(lofasm STATIC
  libintegration/integration.cpp
  libparseraw/parseraw.cpp
  liblofasmfileheader/lofasmfileheader.cpp
  libtime/time.cpp
  liblofasmreader/lofasmreader.cpp)

# add individual programs
#add_subdirectory(readlofasm)
add_subdirectory(lofasm2bbx)
add_subdirectory(readlofasm2)
