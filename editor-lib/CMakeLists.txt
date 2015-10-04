# Find and setup Qt5
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DQT_NO_DEBUG)
endif()

include_directories(editor-lib ${Qt5Core_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})

# Source
file(GLOB_RECURSE src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h)
list(REMOVE_ITEM src static.cpp)
list(INSERT src 0 static.cpp)

# Add resources
file(GLOB_RECURSE qrc RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qrc)

# figure out what to MOC
file(GLOB_RECURSE moc RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
list(REMOVE_ITEM moc static.h)

# manually MOC the files so they can use the precompiled headers
qt5_wrap_cpp(processed_src ${moc})

# manually RCC the files because CMake doesn't automatically re-run
qt5_add_resources(processed_src ${qrc})

# Create library
add_library(editor-lib ${src} ${processed_src})
target_link_libraries(editor-lib LINK_PUBLIC core Qt5::Core Qt5::Widgets ${OS-LIBRARIES})

# Precompiled headers
set_target_properties(editor-lib PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "static.h")
set_target_properties(editor-lib PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
if(Qt5_POSITION_INDEPENDENT_CODE)
  set_target_properties(editor-lib PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
endif()
cotire(editor-lib)

# nice grouping for the IDE
foreach(FILE ${src}) 
  get_filename_component(PARENT_DIR "src/${FILE}" PATH)

  # change /'s to \\'s
  string(REGEX REPLACE "(\\./)/?" "" GROUP "${PARENT_DIR}")
  string(REPLACE "/" "\\" GROUP "${GROUP}")

  source_group("${GROUP}" FILES "${FILE}")
endforeach()

source_group(Generated FILES ${processed_src})