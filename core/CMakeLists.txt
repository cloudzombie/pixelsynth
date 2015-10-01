file(GLOB_RECURSE src ./*.cpp ./*.h)
list(REMOVE_ITEM src ${CMAKE_CURRENT_SOURCE_DIR}/./static.cpp)
list(INSERT src 0 ${CMAKE_CURRENT_SOURCE_DIR}/./static.cpp)
source_group(src FILES ${src})
add_library(core ${src})

# Precompiled headers
set_target_properties(core PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "static.h")
set_target_properties(core PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
cotire(core)
