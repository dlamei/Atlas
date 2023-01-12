cmake_minimum_required(VERSION 3.10)

project(Atlas)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_VERBOSE_MAKEFILE 1)

set(RES_DIR assets)

add_subdirectory(vendor)

set(SOURCES
	src/logger.cpp
	src/window.cpp
	src/gl_utils.cpp
	src/imgui_build.cpp
	src/application.cpp
	src/camera.cpp
	src/atl_types.cpp
	src/RenderApi.cpp
	src/Render2D.cpp

	src/gl_utils.h
	src/gl_atl_utils.h
	src/pch.h
	src/imgui_build.h
	)

set(INCLUDE_FILES
	include/event.h
	include/window.h
	include/utils.h
	include/camera.h
	include/application.h
	include/atl_types.h
	include/RenderApi.h
	include/Render2D.h

	)

set(EMBEDED
	embed/font.embed
)

include_directories(include)
include_directories(embed)

add_executable(${PROJECT_NAME} src/main.cpp
	${SOURCES}
	${INCLUDE_FILES}
	${EMBEDED}
)

target_precompile_headers(${PROJECT_NAME}
	PRIVATE src/pch.h
	)

target_link_libraries(${PROJECT_NAME}
	PRIVATE 
	glad
	glfw
	spdlog
	imgui
	glm
	stb
	)

if (WIN32)
	get_filename_component(real_path "${RES_DIR}" REALPATH)
	string(REPLACE "/" "\\" target_path "${real_path}")

	add_custom_command( #link_resources ALL
		TARGET ${PROJECT_NAME}
		COMMAND cmd /C if not exist "${RES_DIR}" mklink /J ${RES_DIR} "${target_path}"
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
		)

	set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

else()
	add_custom_command(
		TARGET ${PROJECT_NAME}
		COMMAND 
		${CMAKE_COMMAND} -E create_symlink 
		${CMAKE_SOURCE_DIR}/${RES_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>/${RES_DIR}
		)
endif()

