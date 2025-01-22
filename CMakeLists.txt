cmake_minimum_required(VERSION 3.20)
project(vector-accumulator)


# language configuration
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(AGGRESSIVE_OPTIMIZATIONS "-march=native" "-ftree-vectorize")

find_package(benchmark REQUIRED)
find_package(Python3 REQUIRED)


add_compile_definitions()


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


# support for ispc compiler
if (NOT DEFINED ISPC_EXECUTABLE)
	find_program(ISPC_EXECUTABLE ispc)

	if (NOT ISPC_EXECUTABLE)
		message(STATUS "Failed to find ispc compiler, will build anyways")
	else ()
		message(STATUS "Found ispc executable at ${ISPC_EXECUTABLE}")
	endif()
endif()


if (DEFINED ISPC_EXECUTABLE)
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ispc.cmake)
	add_compile_definitions(ISPC_SUPPORT)
endif()


enable_testing()


# experiment enumeration
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB EXPERIMENTS LIST_DIRECTORIES TRUE "${SRC_DIR}/*")


foreach (EXPERIMENT_DIR ${EXPERIMENTS})
	get_filename_component(EXPERIMENT_NAME ${EXPERIMENT_DIR} NAME)
	set(TEST_NAME "${EXPERIMENT_NAME}_test")
	set(BENCH_NAME "${EXPERIMENT_NAME}_bench")
	set(JSON_REPORT_NAME "${EXPERIMENT_NAME}_report.json")

	add_executable(
		${TEST_NAME}
		${EXPERIMENT_DIR}/test.cpp
		${EXPERIMENT_DIR}/${EXPERIMENT_NAME}.ispc
	)
	target_compile_options(${TEST_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:${ARCH_FLAG} ${AGGRESSIVE_OPTIMIZATIONS}>)

	add_executable(
		${BENCH_NAME}
		${EXPERIMENT_DIR}/bench.cpp
		${EXPERIMENT_DIR}/${EXPERIMENT_NAME}.ispc
	)
	target_compile_options(${BENCH_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:${ARCH_FLAG} ${AGGRESSIVE_OPTIMIZATIONS}>)
	target_link_libraries(${BENCH_NAME} PRIVATE benchmark::benchmark)

	add_custom_command(
		OUTPUT ${JSON_REPORT_NAME}
		DEPENDS ${BENCH_NAME}
		COMMAND ${BENCH_NAME} --benchmark_out=${JSON_REPORT_NAME} --benchmark_out_format=json
	)

	add_custom_target(
		"benchmark_${EXPERIMENT_NAME}"
		DEPENDS ${JSON_REPORT_NAME}
		COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/plot.py ${JSON_REPORT_NAME}
	)

	add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
