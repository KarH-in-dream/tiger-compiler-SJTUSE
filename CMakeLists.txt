cmake_minimum_required(VERSION 3.5.0)
project(lab5-warmup VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 20)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(llvm_libs support core irreader)

add_executable(genCalculateDistance genCalculateDistance.cpp)

target_link_libraries(genCalculateDistance ${llvm_libs})

add_executable(genEasy genEasy.cpp)

target_link_libraries(genEasy ${llvm_libs})
