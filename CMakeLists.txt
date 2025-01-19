cmake_minimum_required(VERSION 3.20)
project(vector-accumulator)


# language configuration
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 23)


# support for C++ std::experimental
if (NOT DEFINED COMPILE_SIMD)
	message(STATUS "Checking compiler for experimental simd support")
	try_compile(COMPILE_SIMD ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/simd.cpp)

	if (COMPILE_SIMD)
		message(STATUS "Checking compiler for experimental simd support -- ok")
	else()
		message(STATUS "Checking compiler for experimental simd support -- not supported, some features will be ignored")
	endif()
endif()

if (COMPILE_SIMD)
	add_compile_definitions(EXPERIMENTAL_SIMD_SUPPORT)
endif()


# support for intel ispc
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ispc.cmake)

if (NOT DEFINED ISPC_EXECUTABLE)
	find_program(ISPC_EXECUTABLE ispc)

	if (NOT ISPC_EXECUTABLE)
		message(STATUS "Failed to find ispc compiler, will build anyways")
	else ()
		message(STATUS "Found ispc executable at ${ISPC_EXECUTABLE}")
		add_compile_definitions(ISPC_SUPPORT)
	endif()
endif()



# experiment enumeration
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(DOT_PRODUCT_DIR "${SRC_DIR}/dot")


add_executable(
	dot_product_test
	${DOT_PRODUCT_DIR}/test.cpp
	${DOT_PRODUCT_DIR}/dot.ispc
)
if (UNIX)
	set(arch_flag "-m${ISPC_ARCH_BIT}")
	target_compile_options(dot_product_test PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:${arch_flag}>)
endif()
