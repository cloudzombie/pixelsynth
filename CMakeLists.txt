project(Pixelsynth)
cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_STANDARD 14)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(cmake/cotire/cmake/cotire.cmake)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libs/bandit)
include_directories(${CMAKE_SOURCE_DIR}/libs/cereal/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/glm)
include_directories(${CMAKE_SOURCE_DIR}/libs/tree)
include_directories(${CMAKE_SOURCE_DIR}/libs/variant/include)

add_subdirectory(core)
add_subdirectory(tests)
