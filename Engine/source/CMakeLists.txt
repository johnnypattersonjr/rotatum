# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(ENGINE_DIR ${CMAKE_CURRENT_LIST_DIR})

function(add_engine_src_dir target)
	set(dirs)
	foreach(dir IN ITEMS ${ARGN})
		list(
			APPEND dirs
			${ENGINE_DIR}/${dir}/*.asm
			${ENGINE_DIR}/${dir}/*.c
			${ENGINE_DIR}/${dir}/*.cpp
			${ENGINE_DIR}/${dir}/*.h
			${ENGINE_DIR}/${dir}/*.hpp
		)
	endforeach()
	file(
		GLOB files
		LIST_DIRECTORIES false
		${dirs}
	)
	target_sources(${target} INTERFACE ${files})
endfunction()

if(MSVC AND ((NOT WINSDK_UM_INCLUDE_DIR) OR (NOT WINSDK_UM_LIB_DIR)))
	# We want to prefer libraries in the Windows SDK, so we have to locate it here.

	string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" arch)

	if(arch STREQUAL win32)
		set(arch x86)
	endif()

	set(VS_DIR ${CMAKE_CXX_COMPILER})
	file(TO_CMAKE_PATH "${VS_DIR}" VS_DIR)
	string(REGEX REPLACE /VC/.+$ "" VS_DIR ${VS_DIR})

	find_file(WINSDK_COMMAND
		NAMES
			VcVarsQueryRegistry.bat
			winsdk.bat
		PATH_SUFFIXES
			vsdevcmd/core
		PATHS
			${VS_DIR}/Common7/Tools
		REQUIRED
	)
	mark_as_advanced(WINSDK_COMMAND)

	execute_process(
		COMMAND
			${WINSDK_COMMAND} & cmd.exe /C echo %WindowsSDKDir%
		OUTPUT_VARIABLE
			WindowsSDKDir
		COMMAND_ERROR_IS_FATAL
			ANY
	)

	string(STRIP ${WindowsSDKDir} WindowsSDKDir)
	string(FIND ${WindowsSDKDir} "\n" secondToLastLine REVERSE)
	if(NOT secondToLastLine EQUAL -1)
		math(EXPR secondToLastLine "${secondToLastLine} + 1")
		string(SUBSTRING ${WindowsSDKDir} ${secondToLastLine} -1 WindowsSDKDir)
	endif()

	find_path(WINSDK_UM_INCLUDE_DIR
		NAMES
			windows.h
		PATH_SUFFIXES
			${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um
		PATHS
			${WindowsSDKDir}/Include
		REQUIRED
	)
	mark_as_advanced(WINSDK_UM_INCLUDE_DIR)

	find_path(WINSDK_UM_LIB_DIR
		NAMES
			User32.Lib
		PATH_SUFFIXES
			${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um/${arch}
		PATHS
			${WindowsSDKDir}/Lib
		REQUIRED
	)
	mark_as_advanced(WINSDK_UM_LIB_DIR)
endif()

include(advancedLighting.cmake)
include(basicLighting.cmake)
include(core.cmake)
include(collada.cmake)
include(t3d.cmake)
include(theora.cmake)

if(ENGINE_PLATFORM_WIN32)
	include(dsound.cmake)
endif()

if(ENGINE_PLATFORM_MAC OR ENGINE_PLATFORM_WIN32)
	include(openal.cmake)
endif()
