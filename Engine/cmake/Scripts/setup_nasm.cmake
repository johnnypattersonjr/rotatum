# Copyright (c) Johnny Patterson
#
# SPDX-License-Identifier: GPL-3.0-or-later

file(REAL_PATH ${CMAKE_CURRENT_LIST_DIR}/../.. _setup_nasm_root)

function(setup_nasm)

if(NOT CMAKE_ASM_NASM_COMPILER)
	if(CMAKE_SYSTEM_NAME STREQUAL Windows)
		set(CMAKE_ASM_NASM_COMPILER
			${_setup_nasm_root}/bin/nasm/win32/nasm.exe
			PARENT_SCOPE
		)
	endif()
endif()

endfunction()
