# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(MODULE_NAME ${PROJECT_NAME}-dsound)

string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" arch)
if(arch STREQUAL win32)
	set(arch x86)
endif()

find_path(DIRECTSOUND_INCLUDE_DIR
	NAMES
		dsound.h
	PATHS
		${WINSDK_UM_INCLUDE_DIR}
		$ENV{DXSDK_DIR}/Include
	REQUIRED
)
find_library(DIRECTSOUND_LIBRARY
	NAMES
		dsound.lib
	PATHS
		${WINSDK_UM_LIB_DIR}
		$ENV{DXSDK_DIR}/Lib/${arch}
	REQUIRED
)
mark_as_advanced(DIRECTSOUND_INCLUDE_DIR)
mark_as_advanced(DIRECTSOUND_LIBRARY)

find_path(XAUDIO2_INCLUDE_DIR
	NAMES
		xaudio2.h
	HINTS
		${WINSDK_UM_INCLUDE_DIR}
		$ENV{DXSDK_DIR}/Include
	REQUIRED
)
find_library(XAUDIO2_LIBRARY
	NAMES
		XAudio2.lib
		X3DAudio.lib
	HINTS
		${WINSDK_UM_LIB_DIR}
		$ENV{DXSDK_DIR}/Lib/${arch}
	REQUIRED
)
mark_as_advanced(XAUDIO2_INCLUDE_DIR)
mark_as_advanced(XAUDIO2_LIBRARY)

add_library(${MODULE_NAME} INTERFACE)

add_engine_src_dir(
	${MODULE_NAME}
	sfx/dsound
	sfx/xaudio
)

target_include_directories(
	${MODULE_NAME}
	INTERFACE
		$<BUILD_INTERFACE:${ENGINE_DIR}>
		$<INSTALL_INTERFACE:include>
		${DIRECTSOUND_INCLUDE_DIR}
		${XAUDIO2_INCLUDE_DIR}
)

target_link_libraries(
	${MODULE_NAME}
	INTERFACE
		${PROJECT_NAME}-core
		${PROJECT_NAME}-module
		${DIRECTSOUND_LIBRARY}
		${XAUDIO2_LIBRARY}
)

target_link_libraries(
	${PROJECT_NAME}-app
	INTERFACE
		${MODULE_NAME}
)
