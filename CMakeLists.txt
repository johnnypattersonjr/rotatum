# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.25)
project(rotatum)

option(BUILD_TEMPLATES "" ON)
option(BUILD_TOOLS "" ON)

if(BUILD_TEMPLATES)
	add_subdirectory(Templates)
endif()

if(BUILD_TOOLS)
	add_subdirectory(Tools)
endif()
