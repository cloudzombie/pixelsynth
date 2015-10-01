project(Pixelsynth)
cmake_minimum_required(VERSION 3.2)

# Project settings
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Files and folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Binary libraries and compiler setup
include(cmake/cotire/CMake/cotire.cmake)
include(cmake/os-libraries.cmake)
include(cmake/glfw.cmake)
include(cmake/compiler.cmake)

# Header-only libraries
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libs/bandit)
include_directories(${CMAKE_SOURCE_DIR}/libs/cereal/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/glm)
include_directories(${CMAKE_SOURCE_DIR}/libs/tree)
include_directories(${CMAKE_SOURCE_DIR}/libs/variant/include)

# Subprojects
add_subdirectory(core)
add_subdirectory(tests)
