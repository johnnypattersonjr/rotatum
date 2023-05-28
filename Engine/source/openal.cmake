# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(MODULE_NAME ${PROJECT_NAME}-openal)

add_library(${MODULE_NAME} INTERFACE)

add_engine_src_dir(
	${MODULE_NAME}
	sfx/openal
)

target_include_directories(
	${MODULE_NAME}
	INTERFACE
		$<BUILD_INTERFACE:${ENGINE_DIR}>
		$<INSTALL_INTERFACE:include>
)

if(ENGINE_PLATFORM_MAC)
	add_engine_src_dir(
		${MODULE_NAME}
		sfx/openal/mac
	)

	find_library(OPENAL_LIBRARY OpenAL REQUIRED)

	target_link_libraries(
		${MODULE_NAME}
		INTERFACE
			${OPENAL_LIBRARY}
	)
elseif(ENGINE_PLATFORM_WIN32)
	add_engine_src_dir(
		${MODULE_NAME}
		sfx/openal/win32
	)

	target_include_directories(
		${MODULE_NAME}
		INTERFACE
			$<BUILD_INTERFACE:${ENGINE_DIR}/../lib/openal/win32>
	)
endif()

target_link_libraries(
	${MODULE_NAME}
	INTERFACE
		${PROJECT_NAME}-core
		${PROJECT_NAME}-module
)

target_link_libraries(
	${PROJECT_NAME}-app
	INTERFACE
		${MODULE_NAME}
)
