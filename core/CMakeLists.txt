# Source
file(GLOB_RECURSE src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)
list(REMOVE_ITEM src static.cpp)
list(INSERT src 0 static.cpp)

# Git revision
include(${CMAKE_SOURCE_DIR}/cmake/versioninfo.cmake)

# Create lib
source_group(src FILES ${src})
add_library(core ${src})

# Precompiled headers
set_target_properties(core PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "static.h")
set_target_properties(core PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
cotire(core)
