# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.25)
project(libogg)

if(NOT SOURCE_DIR)
	message(FATAL_ERROR "SOURCE_DIR must be defined.")
endif()

set(SIZE16 int16_t)
set(SIZE32 int32_t)
set(SIZE64 int64_t)
set(USIZE16 uint16_t)
set(USIZE32 uint32_t)

set(config_types-file ${CMAKE_CURRENT_BINARY_DIR}/include/ogg/config_types.h)
configure_file(${SOURCE_DIR}/include/ogg/config_types.h.in ${config_types-file} @ONLY)
file(READ ${config_types-file} configure-result)
file(WRITE ${config_types-file} "#include <stdint.h>\n${configure-result}")

file(
	GLOB files
	LIST_DIRECTORIES false
	${SOURCE_DIR}/src/*.c
	${SOURCE_DIR}/include/ogg/*.h
)

source_group(TREE ${SOURCE_DIR} FILES ${files})

add_library(${PROJECT_NAME} ${files})

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
		$<INSTALL_INTERFACE:include>
)
