# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL YES)

# TODO: Investigate overhead of using FetchContent. It seems to take a few seconds per library.
include(FetchContent)

function(setup_lib target dir)
	set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/${dir})
	if(ARGN)
		set(EXTRA_ARGS SOURCE_SUBDIR ${ARGN})
	else()
		set(EXTRA_ARGS)
	endif()
	FetchContent_Declare(
		${target}
		SOURCE_DIR ${SOURCE_DIR}
		${EXTRA_ARGS}
	)
	FetchContent_MakeAvailable(${target})
	set_property(TARGET ${target} PROPERTY FOLDER ThirdParty)
	target_link_libraries(${target} PRIVATE ${PROJECT_NAME}-lib)
endfunction()

set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_INSTALL OFF)

setup_lib(convexDecomp convexDecomp ../cmake/convexDecomp)
setup_lib(glfw glfw3)
setup_lib(libjpeg ljpeg ../cmake/libjpeg)
setup_lib(libogg libogg ../cmake/libogg)
setup_lib(libungif lungif ../cmake/libungif)
setup_lib(opcode opcode ../cmake/opcode)
setup_lib(pcre pcre ../cmake/pcre)
setup_lib(squish squish ../cmake/squish)
setup_lib(tinyxml tinyxml ../cmake/tinyxml)
setup_lib(zlib zlib ../cmake/zlib)

setup_lib(libpng lpng ../cmake/libpng) # Depends on: zlib
setup_lib(libmng lmng ../cmake/libmng) # Depends on: libjpeg, libpng, zlib
setup_lib(libtheora libtheora ../cmake/libtheora) # Depends on: libogg
setup_lib(libvorbis libvorbis ../cmake/libvorbis) # Depends on: libogg
setup_lib(collada collada ../cmake/collada) # Depends on: pcre, tinyxml
