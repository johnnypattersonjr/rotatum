# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

# TODO: Investigate overhead of using FetchContent. It seems to take a few seconds per library.
include(FetchContent)

function(setup_lib target dir)
	set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/${dir})
	FetchContent_Declare(
		${target}
		SOURCE_DIR ${SOURCE_DIR}
		SOURCE_SUBDIR ../cmake/${target}
	)
	FetchContent_MakeAvailable(${target})
	set_property(TARGET ${target} PROPERTY FOLDER ThirdParty)
	target_link_libraries(${target} PRIVATE ${PROJECT_NAME}-lib)
endfunction()

setup_lib(convexDecomp convexDecomp)
setup_lib(libjpeg ljpeg)
setup_lib(libogg libogg)
setup_lib(libungif lungif)
setup_lib(opcode opcode)
setup_lib(pcre pcre)
setup_lib(squish squish)
setup_lib(tinyxml tinyxml)
setup_lib(zlib zlib)

setup_lib(libpng lpng) # Depends on: zlib
setup_lib(libmng lmng) # Depends on: libjpeg, libpng, zlib
setup_lib(libtheora libtheora) # Depends on: libogg
setup_lib(libvorbis libvorbis) # Depends on: libogg
setup_lib(collada collada) # Depends on: pcre, tinyxml
