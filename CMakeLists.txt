# ---- CMakeLists.txt ----
# Place this in a separate file named CMakeLists.txt next to your .cpp source file.

# Minimum CMake version
cmake_minimum_required(VERSION 3.13)
project(SproutLLVM LANGUAGES CXX)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM libs: ${LLVM_AVAILABLE_LIBS}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Executable
add_executable(sprout main.cpp)

# Link with LLVM
llvm_map_components_to_libnames(LLVM_LIBS
    core
    orcjit
    native
    support
)

target_link_libraries(sprout PRIVATE ${LLVM_LIBS})
