include(ExternalProject)

SET(glfw_CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
	-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DGLFW_BUILD_EXAMPLES=OFF
    -DGLFW_BUILD_TESTS=OFF
    -DGLFW_BUILD_DOCS=OFF
    -Wno-dev
)

EXTERNALPROJECT_ADD(dependency_glfw
    DOWNLOAD_COMMAND ""
    INSTALL_DIR libs
    CMAKE_ARGS ${glfw_CMAKE_ARGS} -DCMAKE_PREFIX_PATH=${install_dir}
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/libs/glfw
)

set_target_properties(dependency_glfw PROPERTIES FOLDER "libs") 

add_library(glfw STATIC IMPORTED)
set_property(TARGET glfw PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/libs/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX})