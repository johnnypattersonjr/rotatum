# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.25)
project(libvorbis)

if(NOT SOURCE_DIR)
	message(FATAL_ERROR "SOURCE_DIR must be defined.")
endif()

file(
	GLOB files
	LIST_DIRECTORIES false
	${SOURCE_DIR}/include/vorbis/*.h
	${SOURCE_DIR}/lib/*.c
	${SOURCE_DIR}/lib/*.h
	#${SOURCE_DIR}/lib/books/coupled/*.h
	#${SOURCE_DIR}/lib/books/floor/*.h
	#${SOURCE_DIR}/lib/books/uncoupled/*.h
	#${SOURCE_DIR}/lib/modes/*.h
)

source_group(TREE ${SOURCE_DIR} FILES ${files})

add_library(${PROJECT_NAME} ${files})

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		$<BUILD_INTERFACE:${SOURCE_DIR}/lib>
	PUBLIC
		$<BUILD_INTERFACE:${SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC
		libogg
)
