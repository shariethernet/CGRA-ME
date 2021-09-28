cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 14)
set (BUILD_SHARED_LIBS ON)

project(
  cgra-me
  VERSION 1.0.0
  LANGUAGES CXX
  )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CGRAME_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)

set(CGRAME_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(PUGIXML_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml)
set(SCIP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/scipoptsuite/scip-4.0.0/src)

add_subdirectory(thirdparty)
add_subdirectory(src)

option (BUILD_LLVM_PASSES
"Build Loop extraction llvm passes. Requires LLVM" OFF)

option (USE_ADL1
"Use new parser for ADL" ON)

if(BUILD_LLVM_PASSES)
  add_subdirectory(llvm-passes)
endif(BUILD_LLVM_PASSES)

