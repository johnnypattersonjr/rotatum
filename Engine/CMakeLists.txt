# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.25)
project(${ENGINE_NAME})

option(ENGINE_TOOL_BUILD "" ON)

if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
	set(ENGINE_PLATFORM_MAC TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
	set(ENGINE_PLATFORM_LINUX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL Windows)
	set(ENGINE_PLATFORM_WIN32 TRUE)
endif()

include(cmake/Scripts/setup_nasm.cmake)
setup_nasm()
enable_language(ASM_NASM)

add_library(${PROJECT_NAME}-app INTERFACE)
add_library(${PROJECT_NAME}-common INTERFACE)
add_library(${PROJECT_NAME}-module INTERFACE)
add_library(${PROJECT_NAME}-lib INTERFACE)

target_link_libraries(
	${PROJECT_NAME}-app
	INTERFACE
		${PROJECT_NAME}-module
)

target_link_libraries(
	${PROJECT_NAME}-lib
	INTERFACE
		${PROJECT_NAME}-common
)

target_link_libraries(
	${PROJECT_NAME}-module
	INTERFACE
		${PROJECT_NAME}-common
)

if(MSVC)
	target_compile_definitions(
		${PROJECT_NAME}-common
		INTERFACE
			_CRT_SECURE_NO_DEPRECATE
			_CRT_SECURE_NO_WARNINGS
			UNICODE
			$<$<CONFIG:Debug,RelWithDebInfo>:
				D3D_DEBUG_INFO
				TORQUE_DEBUG
				TORQUE_DEBUG_GUARD
				TORQUE_NET_STATS
			>
	)

	target_compile_options(
		${PROJECT_NAME}-common
		INTERFACE
			$<$<COMPILE_LANGUAGE:C,CXX>:
				/GS-         # [ ] Buffer Security Checks
				/Gy          # [+] Function Level Linking
				/GF          # [+] String Pooling
				/Oi          # [+] Intrinsic Functions
				/GR          # [+] Run-Time Type Information
				/Zc:wchar_t- # [ ] Treat wchar_t as Native Type
				$<$<CONFIG:Debug>:
					/Od      # [ ] Optimization
					/ZI      # [+] Edit and Continue Debug
				>
				/wd4244
			>
	)

	target_link_options(
		${PROJECT_NAME}-common
		INTERFACE
			/NOLOGO
			/NODEFAULTLIB:LIBC
			/NODEFAULTLIB:LIBCD
	)

	target_compile_options(
		${PROJECT_NAME}-app
		INTERFACE
			$<$<COMPILE_LANGUAGE:C,CXX>:
				/W4          #     Warning Level: 4
				/wd4018
				/wd4100
				/wd4121
				/wd4127
				/wd4130
				/wd4245
				/wd4389
				/wd4511
				/wd4512
				/wd4800
			>
	)

	target_link_options(
		${PROJECT_NAME}-app
		INTERFACE
			/DYNAMICBASE:NO
			/NODEFAULTLIB:LIBC
			/NODEFAULTLIB:LIBCD
	)

	target_compile_options(
		${PROJECT_NAME}-lib
		INTERFACE
			$<$<COMPILE_LANGUAGE:C,CXX>:
				/W3          #     Warning Level: 3
				/wd4305
				/wd4355
				/wd4530
			>
	)
endif()

add_subdirectory(lib)
add_subdirectory(source)
