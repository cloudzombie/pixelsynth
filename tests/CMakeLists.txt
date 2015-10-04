# Source
file(GLOB_RECURSE src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)
list(REMOVE_ITEM src static.cpp)
list(INSERT src 0 static.cpp)
source_group(src FILES ${src})

# figure out what to MOC
file(GLOB_RECURSE moc RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
list(REMOVE_ITEM moc static.h)

# manually MOC the files so they can use the precompiled headers
qt5_wrap_cpp(processed_src ${moc})

# Create executable
add_executable(tests ${src} ${processed_src})
target_link_libraries(tests LINK_PUBLIC core editor-lib)

# Precompiled headers
set_target_properties(tests PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "static.h")
set_target_properties(tests PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
cotire(tests)

# Grouping
source_group(Generated FILES ${processed_src})
