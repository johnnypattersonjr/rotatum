# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.25)
project(libogg)

if(NOT SOURCE_DIR)
	message(FATAL_ERROR "SOURCE_DIR must be defined.")
endif()

file(
	GLOB files
	LIST_DIRECTORIES false
	${SOURCE_DIR}/src/*.c
	${SOURCE_DIR}/src/*.h
	${SOURCE_DIR}/include/ogg/*.c
	${SOURCE_DIR}/include/ogg/*.h
)

source_group(TREE ${SOURCE_DIR} FILES ${files})

add_library(${PROJECT_NAME} ${files})

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)
