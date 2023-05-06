# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(MODULE_NAME ${PROJECT_NAME}-theora)

add_library(${MODULE_NAME} INTERFACE)

add_engine_src_dir(
	${MODULE_NAME}
	core/ogg
	gfx/video
	gui/theora
)

target_compile_definitions(
	${MODULE_NAME}
	INTERFACE
		TORQUE_OGGTHEORA
		TORQUE_OGGVORBIS
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
		libtheora
		libvorbis
)

target_link_libraries(
	${PROJECT_NAME}-app
	INTERFACE
		${MODULE_NAME}
)
