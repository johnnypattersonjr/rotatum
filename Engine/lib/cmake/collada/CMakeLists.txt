# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.25)
project(collada)

if(NOT SOURCE_DIR)
	message(FATAL_ERROR "SOURCE_DIR must be defined.")
endif()

file(
	GLOB files
	LIST_DIRECTORIES false
	${SOURCE_DIR}/include/1.4/dom/*.h
	${SOURCE_DIR}/include/dae/*.h
	${SOURCE_DIR}/include/modules/LIBXMLPlugin/*.h
	${SOURCE_DIR}/include/modules/stdErrPlugin/*.h
	${SOURCE_DIR}/include/modules/STLDatabase/*.h
	${SOURCE_DIR}/src/1.4/dom/*.cpp
	${SOURCE_DIR}/src/dae/*.cpp
	${SOURCE_DIR}/src/modules/LIBXMLPlugin/*.cpp
	${SOURCE_DIR}/src/modules/stdErrPlugin/*.cpp
	${SOURCE_DIR}/src/modules/STLDatabase/*.cpp
)

source_group(TREE ${SOURCE_DIR} FILES ${files})

add_library(${PROJECT_NAME} ${files})

target_compile_definitions(
	${PROJECT_NAME}
	PUBLIC
		DOM_INCLUDE_TINYXML
)

target_include_directories(
	${PROJECT_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${SOURCE_DIR}/include/1.4>
		$<INSTALL_INTERFACE:include>
		$<INSTALL_INTERFACE:include/1.4>
)

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC
		pcre
		tinyxml
)
