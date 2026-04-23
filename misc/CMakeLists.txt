cmake_minimum_required(VERSION 3.16)
project(MyOneAPIProject CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find DPC++ compiler (icpx)
if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER icpx)
endif()

# oneAPI include and lib paths (adjust if needed)
set(ONEAPI_ROOT /opt/intel/oneapi)
include_directories(
    ${ONEAPI_ROOT}/compiler/latest/include
    ${ONEAPI_ROOT}/ishmem/latest/include
)
link_directories(
    ${ONEAPI_ROOT}/compiler/latest/lib
    ${ONEAPI_ROOT}/ishmem/latest/lib
)

# Source files
add_executable(MyOneAPIProject PracticePad.cpp)

# Link libraries
target_link_libraries(MyOneAPIProject
    ishmem
    sycl
    ze_loader
)