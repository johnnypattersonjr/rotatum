# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.25)
project(convexDecomp)

if(NOT SOURCE_DIR)
	message(FATAL_ERROR "SOURCE_DIR must be defined.")
endif()

file(
	GLOB files
	LIST_DIRECTORIES false
	${SOURCE_DIR}/*.cpp
	${SOURCE_DIR}/*.h
	${SOURCE_DIR}/*.inl
)

source_group(TREE ${SOURCE_DIR} FILES ${files})

add_library(${PROJECT_NAME} ${files})

target_compile_definitions(
	${PROJECT_NAME}
	PUBLIC
		$<$<BOOL:${ENGINE_PLATFORM_MAC}>:__APPLE__>
		$<$<BOOL:${ENGINE_PLATFORM_WIN32}>:WIN32>
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)
