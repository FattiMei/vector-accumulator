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


enable_testing()


# experiment enumeration
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB EXPERIMENTS LIST_DIRECTORIES true "${SRC_DIR}/*")


foreach (EXPERIMENT_DIR ${EXPERIMENTS})
	get_filename_component(EXPERIMENT_NAME ${EXPERIMENT_DIR} NAME)
	set(TEST_NAME "${EXPERIMENT_NAME}_test")

	add_executable(
		${TEST_NAME}
		${EXPERIMENT_DIR}/test.cpp
		${EXPERIMENT_DIR}/${EXPERIMENT_NAME}.ispc
	)
	if (UNIX)
		set(arch_flag "-m${ISPC_ARCH_BIT}")
		target_compile_options(${TEST_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:${arch_flag}>)
		target_compile_options(${TEST_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-march=native>)
	endif()

	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
