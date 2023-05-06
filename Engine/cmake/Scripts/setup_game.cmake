# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

file(REAL_PATH ${CMAKE_CURRENT_LIST_DIR}/../.. _setup_game_root)

function(setup_game PROJECT_NAME ENGINE_NAME)

set_property(
	TARGET ${PROJECT_NAME}
	PROPERTY RUNTIME_OUTPUT_DIRECTORY
	${CMAKE_CURRENT_LIST_DIR}/game
)
foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
	string(TOUPPER ${config} config)
	set_property(
		TARGET ${PROJECT_NAME}
		PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config}
		${CMAKE_CURRENT_LIST_DIR}/game
	)
endforeach()

file(GLOB_RECURSE engine-sources ${_setup_game_root}/source/*)
source_group(
	TREE ${_setup_game_root}/source
	FILES ${engine-sources}
	PREFIX Engine
)
file(GLOB_RECURSE game-sources ${CMAKE_CURRENT_LIST_DIR}/source/*)
source_group(
	TREE ${CMAKE_CURRENT_LIST_DIR}/source
	FILES ${game-sources}
	PREFIX Game
)
file(GLOB game-root-files LIST_DIRECTORIES FALSE ${CMAKE_CURRENT_LIST_DIR}/*)
source_group("\\" FILES ${game-root-files})

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		${ENGINE_NAME}-app
)

endfunction()
