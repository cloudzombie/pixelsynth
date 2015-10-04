file(GLOB_RECURSE src ./*.cpp ./*.h)
source_group(src FILES ${src})
add_executable(editor ${src})
target_link_libraries(editor LINK_PUBLIC core editor-lib)