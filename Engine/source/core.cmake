# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(MODULE_NAME ${PROJECT_NAME}-core)

string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" arch)
if(arch STREQUAL win32)
	set(arch x86)
endif()

add_library(${MODULE_NAME} INTERFACE)

add_engine_src_dir(
	${MODULE_NAME}
	app
	app/net
	component
	component/interfaces
	console
	core
	core/stream
	core/strings
	core/util
	core/util/journal
	core/util/journal/test
	core/util/test
	core/util/zip
	core/util/zip/compressors
	core/util/zip/unitTests
	gfx
	gfx/bitmap
	gfx/bitmap/loaders
	gfx/Null
	gfx/sim
	gfx/test
	gfx/util
	gfx/video
	gui
	gui/buttons
	gui/containers
	gui/controls
	gui/core
	gui/game
	gui/shiny
	gui/utility
	i18n
	math
	math/test
	math/util
	platform
	platform/async
	platform/threads
	sfx
	sfx/media
	sfx/null
	shaderGen
	sim
	unit
	unit/tests
	util
	util/messaging
	windowManager
	windowManager/glfw
	windowManager/test
)

if(ENGINE_PLATFORM_LINUX OR ENGINE_PLATFORM_MAC OR ENGINE_PLATFORM_WIN32)
	add_engine_src_dir(
		${MODULE_NAME}
		platform/menus
		platform/nativeDialogs
		platform/test
	)
endif()

if(ENGINE_PLATFORM_LINUX)
	add_engine_src_dir(
		${MODULE_NAME}
		platformPOSIX
		platformX86UNIX
		platformX86UNIX/threads
	)
elseif(ENGINE_PLATFORM_MAC)
	add_engine_src_dir(
		${MODULE_NAME}
		gfx/gl
		gfx/gl/ggl
		gfx/gl/ggl/generated
		gfx/gl/ggl/mac
		platformMac
		platformMac/Menus
		platformPOSIX
		shaderGen/glsl
		windowManager/mac
	)

	find_library(AGL_LIBRARY AGL REQUIRED)
	find_library(CARBON_LIBRARY Carbon REQUIRED)
	find_library(COCOA_LIBRARY Cocoa REQUIRED)
	find_library(OPENGL_LIBRARY OpenGL REQUIRED)

	target_link_libraries(
		${MODULE_NAME}
		INTERFACE
			${AGL_LIBRARY}
			${CARBON_LIBRARY}
			${COCOA_LIBRARY}
			${OPENGL_LIBRARY}
	)
elseif(ENGINE_PLATFORM_WIN32)
	add_engine_src_dir(
		${MODULE_NAME}
		platformWin32
		platformWin32/menus
		platformWin32/minidump
		platformWin32/nativeDialogs
		platformWin32/threads
		platformWin32/videoInfo
		shaderGen/hlsl
		windowManager/win32
	)

	target_compile_definitions(
		${MODULE_NAME}
		INTERFACE
			_CRT_SECURE_NO_DEPRECATE
			INITGUID
			UNICODE
	)

	# TEMP: We are preferring the DirectX SDK over the Windows SDK
	#       until we no longer rely on d3dx.

	find_path(D3D9_INCLUDE_DIR
		NAMES
			d3d9.h
		PATHS
			$ENV{DXSDK_DIR}/Include
			${WINSDK_UM_INCLUDE_DIR}
		REQUIRED
	)
	find_library(D3D9_LIBRARY
		NAMES
			d3d9.lib
		PATHS
			$ENV{DXSDK_DIR}/Lib/${arch}
			${WINSDK_UM_LIB_DIR}
		REQUIRED
	)
	mark_as_advanced(D3D9_INCLUDE_DIR)
	mark_as_advanced(D3D9_LIBRARY)

	if(D3D9_INCLUDE_DIR)
		# DxErr and d3dx9 are deprecated in later Windows SDK versions.
		# We should only use them when using an SDK that includes them.
		# d3d9 is also deprecated in later Windows SDK versions, so we
		# need to investigate how to handle it properly in those cases.

		find_path(D3DX9_INCLUDE_DIR
			NAMES
				d3dx9.h
			PATHS
				$ENV{DXSDK_DIR}/Include
				${WINSDK_UM_INCLUDE_DIR}
		)
		find_library(D3DX9_LIBRARY
			NAMES
				d3dx9.lib
			PATHS
				$ENV{DXSDK_DIR}/Lib/${arch}
				${WINSDK_UM_LIB_DIR}
		)
		find_path(DXERR_INCLUDE_DIR
			NAMES
				DxErr.h
			PATHS
				$ENV{DXSDK_DIR}/Include
				${WINSDK_UM_INCLUDE_DIR}
		)
		find_library(DXERR_LIBRARY
			NAMES
				DxErr.lib
			PATHS
				$ENV{DXSDK_DIR}/Lib/${arch}
				${WINSDK_UM_LIB_DIR}
		)
		mark_as_advanced(D3DX9_INCLUDE_DIR)
		mark_as_advanced(D3DX9_LIBRARY)
		mark_as_advanced(DXERR_INCLUDE_DIR)
		mark_as_advanced(DXERR_LIBRARY)

		add_engine_src_dir(
			${MODULE_NAME}
			gfx/D3D9
			gfx/D3D9/pc
		)

		target_include_directories(
			${MODULE_NAME}
			INTERFACE
				${D3D9_INCLUDE_DIR}
		)

		target_link_libraries(
			${MODULE_NAME}
			INTERFACE
				${D3D9_LIBRARY}
		)

		if(D3DX9_INCLUDE_DIR STREQUAL D3D9_INCLUDE_DIR)
			target_link_libraries(
				${MODULE_NAME}
				INTERFACE
					${D3DX9_LIBRARY}
			)

			target_compile_definitions(
				${MODULE_NAME}
				INTERFACE
					HAVE_D3DX9
			)
		endif()

		if(DXERR_INCLUDE_DIR STREQUAL D3D9_INCLUDE_DIR)
			target_link_libraries(
				${MODULE_NAME}
				INTERFACE
					${DXERR_LIBRARY}
			)

			target_compile_definitions(
				${MODULE_NAME}
				INTERFACE
					HAVE_DXERR
			)
		endif()
	endif()

	target_link_libraries(
		${MODULE_NAME}
		INTERFACE
			ComCtl32.Lib
			ComDlg32.Lib
			User32.Lib
			AdvAPI32.Lib
			Gdi32.Lib
			WinMM.Lib
			WSock32.Lib
			Vfw32.Lib
			Imm32.Lib
			Ole32.Lib
			shell32.lib
			OleAut32.Lib
			Version.Lib
	)
endif()

if(ENGINE_TOOL_BUILD)
	add_engine_src_dir(
		${MODULE_NAME}
		environment/editors
		forest/editors
		gui/editor
		gui/editor/inspector
		gui/worldEditor
	)
else()
	target_compile_definitions(
		${MODULE_NAME}
		INTERFACE
			TORQUE_PLAYER
	)
endif()

target_compile_definitions(
	${MODULE_NAME}
	INTERFACE
		TORQUE_COLLADA
		TORQUE_SHADERGEN
		TORQUE_UNICODE
)

target_include_directories(
	${MODULE_NAME}
	INTERFACE
		$<BUILD_INTERFACE:${ENGINE_DIR}>
		$<INSTALL_INTERFACE:include>
)

target_link_libraries(
	${MODULE_NAME}
	INTERFACE
		${PROJECT_NAME}-module
		collada
		convexDecomp
		glfw
		libjpeg
		libmng
		libpng
		libungif
		opcode
		pcre
		squish
		tinyxml
		zlib
)

target_link_libraries(
	${PROJECT_NAME}-app
	INTERFACE
		${MODULE_NAME}
)
