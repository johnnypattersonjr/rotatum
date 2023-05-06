# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.25)
project(Full)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)
endif()

include(../../Engine/cmake/Scripts/setup_game.cmake)
include(../../Engine/cmake/Scripts/setup_nasm.cmake)

setup_nasm()
enable_language(ASM_NASM)

include_directories(source)

set(ENGINE_NAME ${PROJECT_NAME})
set(ENGINE_HIFI_NET FALSE)
add_subdirectory(../../Engine Engine)

add_executable(${PROJECT_NAME} ../../Engine/source/main/main.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER Templates)

setup_game(${PROJECT_NAME} ${ENGINE_NAME})

if(WIN32)
	set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
endif()
