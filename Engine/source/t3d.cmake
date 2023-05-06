# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(MODULE_NAME ${PROJECT_NAME}-t3d)

add_library(${MODULE_NAME} INTERFACE)

add_engine_src_dir(
	${MODULE_NAME}
	collision
	environment
	forest
	forest/editor
	forest/ts
	gui/3d
	interior
	lighting
	lighting/common
	materials
	physics
	postFx
	renderInstance
	scene
	scene/culling
	scene/mixin
	scene/zones
	shaderGen
	T3D
	T3D/decal
	T3D/examples
	T3D/fps
	T3D/fx
	T3D/gameBase
	T3D/physics
	T3D/sfx
	T3D/turret
	T3D/vehicles
	terrain
	ts
	ts/arch
)

if(TORQUE_HIFI_NET)
	add_engine_src_dir(
		${MODULE_NAME}
		T3D/gameBase/hifi
	)

	target_compile_definitions(
		${MODULE_NAME}
		INTERFACE
			TORQUE_HIFI_NET
	)
else()
	add_engine_src_dir(
		${MODULE_NAME}
		T3D/gameBase/std
	)
endif()

if(ENGINE_PLATFORM_WIN32)
	add_engine_src_dir(
		${MODULE_NAME}
		terrain/hlsl
		forest/hlsl
	)
else()
	add_engine_src_dir(
		${MODULE_NAME}
		terrain/glsl
		forest/glsl
	)
endif()

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
		collada
		opcode
		tinyxml
)

target_link_libraries(
	${PROJECT_NAME}-app
	INTERFACE
		${MODULE_NAME}
)
