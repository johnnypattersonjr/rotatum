# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.25)
project(libmng)

if(NOT SOURCE_DIR)
	message(FATAL_ERROR "SOURCE_DIR must be defined.")
endif()

file(
	GLOB files
	LIST_DIRECTORIES false
	${SOURCE_DIR}/*.c
	${SOURCE_DIR}/*.h
)

source_group(TREE ${SOURCE_DIR} FILES ${files})

add_library(${PROJECT_NAME} ${files})

target_compile_definitions(
	${PROJECT_NAME}
	PUBLIC
		MNG_OPTIMIZE_OBJCLEANUP
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC
		libjpeg
		libpng
		zlib
)
