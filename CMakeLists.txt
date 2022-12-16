cmake_minimum_required(VERSION 3.10)

project(Atlas)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_VERBOSE_MAKEFILE 1)

add_subdirectory(vendor)

set(SOURCES
	src/logger.cpp
	src/window.cpp
	)

set(HEADERS
	include/event.h
	include/window.h
	)

include_directories(include)

add_executable(${PROJECT_NAME} src/main.cpp
	${SOURCES}
	${HEADERS}
)

target_precompile_headers(${PROJECT_NAME}
	PRIVATE src/pch.h
	)

target_link_libraries(${PROJECT_NAME}
	PRIVATE glad glfw spdlog
	)
