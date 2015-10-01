find_package(OpenGL REQUIRED)

if(MSVC)
  set(OS-LIBRARIES ${OPENGL_LIBRARIES})
endif()

if(UNIX)
	if(APPLE)
		find_library(carbon_lib Carbon) # we look for the Carbon framework and use carbon_lib as an alias for it
		find_library(iokit_lib IOKit)
		find_library(forcefeedback_lib ForceFeedback)
		find_library(cocoa_lib Cocoa)
		find_library(audiounit_lib AudioUnit)
		find_library(coreaudio_lib CoreAudio)
		find_library(opengl_lib OpenGL)
		find_library(corefoundation_lib CoreFoundation)
		find_library(corevideo_lib CoreVideo)

		set(OS-LIBRARIES
		    ${carbon_lib}
		    ${iokit_lib}
		    ${forcefeedback_lib}
		    ${cocoa_lib}
		    ${audiounit_lib}
		    ${coreaudio_lib}
		    ${opengl_lib}
		    ${corefoundation_lib}
			${corevideo_lib})
	else()
		set(THREADS_PREFER_PTHREAD_FLAG ON)
		find_package(Threads REQUIRED)
		find_library(rt NAMES rt)
		find_library(x11 NAMES X11)
		find_library(xrandr NAMES Xrandr)
		find_library(xcursor NAMES Xcursor)
		find_library(xi NAMES Xi)
		find_library(xinerama NAMES Xinerama)
		find_library(xxf86vm NAMES Xxf86vm)
	  	set(OS-LIBRARIES Threads::Threads ${rt} ${x11} ${xrandr} ${xcursor} ${xi} ${xinerama} ${xxf86vm} ${CMAKE_DL_LIBS} ${OPENGL_LIBRARIES})
	    add_compile_options(-pthread)
	endif()
endif()

message(STATUS "** Using OS Libraries: ${OS-LIBRARIES}")
