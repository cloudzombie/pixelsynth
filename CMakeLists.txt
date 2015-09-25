cmake_minimum_required(VERSION 3.3)
project(newnodes)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp tree.h node.h node.cpp)
add_executable(newnodes ${SOURCE_FILES})