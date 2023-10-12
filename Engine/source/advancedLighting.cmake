# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(MODULE_NAME ${PROJECT_NAME}-advancedLighting)

add_library(${MODULE_NAME} INTERFACE)

add_engine_src_dir(
	${MODULE_NAME}
	lighting/advanced
	lighting/advanced/glsl
	lighting/shadowMap
)

if(ENGINE_PLATFORM_WIN32)
	add_engine_src_dir(
		${MODULE_NAME}
		lighting/advanced/hlsl
	)
endif()

target_compile_definitions(
	${MODULE_NAME}
	INTERFACE
		TORQUE_ADVANCED_LIGHTING
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
		${PROJECT_NAME}-core
		${PROJECT_NAME}-module
)

target_link_libraries(
	${PROJECT_NAME}-app
	INTERFACE
		${MODULE_NAME}
)
