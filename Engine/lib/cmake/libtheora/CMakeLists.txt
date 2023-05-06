# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.25)
project(libtheora)

if(NOT SOURCE_DIR)
	message(FATAL_ERROR "SOURCE_DIR must be defined.")
endif()

set(input
	${SOURCE_DIR}/include/theora/*.h
	${SOURCE_DIR}/lib/*.c
	${SOURCE_DIR}/lib/*.h
	${SOURCE_DIR}/lib/dec/*.c
	${SOURCE_DIR}/lib/dec/*.h
	${SOURCE_DIR}/lib/enc/*.c
	${SOURCE_DIR}/lib/enc/*.h
	${SOURCE_DIR}/*.c
	${SOURCE_DIR}/*.h
)

if(ENGINE_PLATFORM_WIN32)
	list(APPEND input
		${SOURCE_DIR}/lib/dec/x86_vc/*.c
		${SOURCE_DIR}/lib/dec/x86_vc/*.h
		${SOURCE_DIR}/lib/enc/x86_vs/*.c
		${SOURCE_DIR}/lib/enc/x86_vs/*.h
	)
else()
	list(APPEND input
		${SOURCE_DIR}/lib/dec/x86/*.c
		${SOURCE_DIR}/lib/enc/x86/*.c
	)
endif()

file(
	GLOB files
	LIST_DIRECTORIES false
	${input}
)

source_group(TREE ${SOURCE_DIR} FILES ${files})

add_library(${PROJECT_NAME} ${files})

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC
		libogg
)
