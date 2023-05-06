# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.25)
project(pcre)

if(NOT SOURCE_DIR)
	message(FATAL_ERROR "SOURCE_DIR must be defined.")
endif()

file(
	GLOB files
	LIST_DIRECTORIES false
	${SOURCE_DIR}/*.c
	${SOURCE_DIR}/*.cc
	${SOURCE_DIR}/*.h
)

source_group(TREE ${SOURCE_DIR} FILES ${files})

add_library(${PROJECT_NAME} ${files})

target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE
		HAVE_CONFIG_H
	PUBLIC
		PCRE_STATIC
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)
