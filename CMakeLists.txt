cmake_minimum_required(VERSION 3.20)
project(vector-accumulator)


# language configuration
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 23)

find_package(benchmark REQUIRED)
find_package(Python3 REQUIRED)


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
	set(BENCH_NAME "${EXPERIMENT_NAME}_bench")
	set(JSON_REPORT_NAME "${EXPERIMENT_NAME}_report.json")

	# TODO: the one thing I don't particularly like is the repetition of the compilation commands for the test and the benchmark executables
	# dovrebbero diventare funzioni
	add_executable(
		${TEST_NAME}
		${EXPERIMENT_DIR}/test.cpp
		${EXPERIMENT_DIR}/${EXPERIMENT_NAME}.ispc
	)
	set(arch_flag "-m${ISPC_ARCH_BIT}")
	target_compile_options(${TEST_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:${arch_flag}>)
	target_compile_options(${TEST_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-march=native>)

	add_executable(
		${BENCH_NAME}
		${EXPERIMENT_DIR}/bench.cpp
		${EXPERIMENT_DIR}/${EXPERIMENT_NAME}.ispc
	)
	set(arch_flag "-m${ISPC_ARCH_BIT}")
	target_compile_options(${TEST_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:${arch_flag}>)
	target_compile_options(${TEST_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C,CXX>:-march=native>)
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
