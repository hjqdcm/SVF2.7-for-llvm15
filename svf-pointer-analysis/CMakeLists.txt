# CMakeLists.txt for SVF Pointer Analysis Pass
cmake_minimum_required(VERSION 3.13)
project(SVFPointerAnalysis)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# Find LLVM 14
find_package(LLVM 14 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

# Include LLVM directories
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# SVF root directory
set(SVF_ROOT "/home/gzr/桌面/project/SVF")

# Add SVF include directories
include_directories(
    ${SVF_ROOT}/svf-llvm/include
    ${SVF_ROOT}/svf/include
    ${SVF_ROOT}/build/include
)

# Library paths
set(SVF_BUILD_DIR "${SVF_ROOT}/build")
link_directories(${SVF_BUILD_DIR}/svf ${SVF_BUILD_DIR}/svf-llvm ${LLVM_LIBRARY_DIRS})

# Create Pass library
add_library(SVFPointerAnalysis MODULE
    src/MySVFPointerPass.cpp
)

# Set properties
set_target_properties(SVFPointerAnalysis PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Link libraries
target_link_libraries(SVFPointerAnalysis
    SvfLLVM
    SvfCore
    ${LLVM_LIBRARIES}
    pthread dl z rt
)