#
#  Copyright (c) 2018-2023, Intel Corporation
#
#  SPDX-License-Identifier: BSD-3-Clause
#
#  Modified by @FattiMei to suit his needs


include_guard(DIRECTORY)


# parsing target architecture (mandatory argument for the ispc compiler)
if (UNIX)
	if(NOT ISPC_ARCH)
		execute_process( COMMAND sh "-c" "uname -m | sed -e s/x86_64/x86/ -e s/amd64/x86/ -e s/i686/x86/ -e s/arm64/aarch64/ -e s/arm.*/arm/ -e s/sa110/arm/" OUTPUT_VARIABLE ARCH)

		string(STRIP ${ARCH} ARCH)
		execute_process( COMMAND getconf LONG_BIT OUTPUT_VARIABLE ARCH_BIT)
		string(STRIP ${ARCH_BIT} arch_bit)
		if ("${ARCH}" STREQUAL "x86")
			if (${arch_bit} EQUAL 32)
				set(ispc_arch "x86")
			else()
				set(ispc_arch "x86-64")
			endif()
		elseif ("${ARCH}" STREQUAL "arm")
			set(ispc_arch "arm")
		elseif ("${ARCH}" STREQUAL "aarch64")
			set(ispc_arch "aarch64")
		endif()
	endif()

	set(ISPC_ARCH "${ispc_arch}" CACHE STRING "ISPC CPU ARCH")
	set(ISPC_ARCH_BIT "${arch_bit}" CACHE STRING "ISPC CPU BIT")
else()
	if(NOT ISPC_ARCH)
		set(ispc_arch "x86")
		if (CMAKE_SIZEOF_VOID_P EQUAL 8 )
			set(ispc_arch "x86-64")
		endif()
	endif()

	set(ISPC_ARCH "${ispc_arch}" CACHE STRING "ISPC CPU ARCH")
	set(ISPC_ARCH_BIT "${arch_bit}" CACHE STRING "ISPC CPU BIT")
	set(ARCH_FLAG "-m${ISPC_ARCH_BIT}" CACHE STRING "ISPC ARCH FLAG")
endif()


enable_language(ISPC)
