#----------------------------------------------------------
# targets
#----------------------------------------------------------

file(GLOB_RECURSE TARGET_SRC *.cpp .)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
	set(TARGET_NAME datamodelgen-dbg)
elseif (${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
	set(TARGET_NAME datamodelgen-odbg)
elseif (${CMAKE_BUILD_TYPE} MATCHES "Release")
	set(TARGET_NAME datamodelgen)
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

add_executable(${TARGET_NAME} ${TARGET_SRC})

target_link_libraries(${TARGET_NAME} ${OPENGL_gl_LIBRARY} objective3d${LIB_EXT})

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
		target_link_libraries(${TARGET_NAME} ${OPENGL_gl_LIBRARY} o3dnet-dbg wsock32 ws2_32 objective3d-dbg)
	elseif (${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
		target_link_libraries(${TARGET_NAME} ${OPENGL_gl_LIBRARY} o3dnet-odbg wsock32 ws2_32 objective3d-odbg)
	elseif (${CMAKE_BUILD_TYPE} MATCHES "Release")
		target_link_libraries(${TARGET_NAME} ${OPENGL_gl_LIBRARY} o3dnet wsock32 ws2_32 objective3d)
	endif()
else()
	if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
		target_link_libraries(${TARGET_NAME} objective3d-dbg ${OPENGL_gl_LIBRARY} o3dnet-dbg)
	elseif (${CMAKE_BUILD_TYPE} MATCHES "RelWithDebInfo")
		target_link_libraries(${TARGET_NAME} objective3d-odbg ${OPENGL_gl_LIBRARY} o3dnet-odbg)
	elseif (${CMAKE_BUILD_TYPE} MATCHES "Release")
		target_link_libraries(${TARGET_NAME} objective3d ${OPENGL_gl_LIBRARY} o3dnet)
	endif()
endif()

#----------------------------------------------------------
# install
#----------------------------------------------------------

install (TARGETS ${TARGET_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT library)
