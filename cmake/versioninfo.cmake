# Extract Git revision info
include(${CMAKE_SOURCE_DIR}/cmake/GetGitRevisionDescription.cmake)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
git_describe(GIT_DESCRIBE)
include_directories(${CMAKE_BINARY_DIR}/versioninfo)

# Add to src
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/versioninfo.h.in" "${CMAKE_BINARY_DIR}/versioninfo/versioninfo.h" @ONLY)
list(APPEND src "${CMAKE_BINARY_DIR}/versioninfo/versioninfo.h")
