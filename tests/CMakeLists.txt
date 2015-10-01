file(GLOB_RECURSE src ./*.cpp ./*.h)
source_group(src FILES ${src})
add_executable (tests ${src})
target_link_libraries (tests LINK_PUBLIC core)

# Precompiled headers
set_target_properties(tests PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "static.h")
set_target_properties(tests PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
cotire(tests)
