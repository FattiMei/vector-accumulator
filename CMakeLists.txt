cmake_minimum_required(VERSION 3.20)
project(vector-accumulator CXX ASM)


# language configuration
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(AGGRESSIVE_OPTIMIZATIONS "-march=native" "-ftree-vectorize")

find_package(benchmark REQUIRED)
find_package(Python3 REQUIRED)


# options for the benchmark
option(USE_FLOAT32 "All floating point numbers are fp32" OFF)
option(USE_STATIC_MEMORY "Profile the reductions on static arrays" OFF)

if (USE_FLOAT32)
	add_compile_definitions(USE_FLOAT32)
endif()

if (USE_STATIC_MEMORY)
	add_compile_definitions(USE_STATIC_MEMORY)
endif()


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
	add_compile_definitions(ISPC_SUPPORT)
endif()


enable_testing()


# experiment enumeration
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB EXPERIMENTS LIST_DIRECTORIES TRUE "${SRC_DIR}/*")


foreach (EXPERIMENT_DIR ${EXPERIMENTS})
	# this will be convoluted because I need some sophisticated features:
	#   1. Since both the test target and the experiment target include the same .hpp file with templated implementations, the recompilation of the file is a waste of resources
	#   2. I want to look at the generated assembly like in compiler explorer, don't want to recompile the file same file for an object output and an assembly output, so I'll produce only the assembly
	get_filename_component(EXPERIMENT_NAME ${EXPERIMENT_DIR} NAME)
	set(TEST_EXE_NAME    "${EXPERIMENT_NAME}_test")
	set(BENCH_EXE_NAME   "${EXPERIMENT_NAME}_bench")
	set(JSON_REPORT_NAME "${EXPERIMENT_NAME}_report.json")

	add_executable(${TEST_EXE_NAME} ${EXPERIMENT_DIR}/test.cpp)

	add_executable(${BENCH_EXE_NAME} ${EXPERIMENT_DIR}/bench.cpp)
	target_compile_options(${BENCH_EXE_NAME} PRIVATE ${AGGRESSIVE_OPTIMIZATIONS})
	target_link_libraries(${BENCH_EXE_NAME} PRIVATE benchmark::benchmark)

	add_custom_command(
		OUTPUT ${JSON_REPORT_NAME}
		DEPENDS ${BENCH_EXE_NAME}
		COMMAND ${BENCH_EXE_NAME} --benchmark_out=${JSON_REPORT_NAME} --benchmark_out_format=json
	)

	add_custom_target(
		"benchmark_${EXPERIMENT_NAME}"
		DEPENDS ${JSON_REPORT_NAME}
		COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/plot.py ${JSON_REPORT_NAME}
	)

	# assembly generation of the ISPC implementations
	if (DEFINED ISPC_EXECUTABLE)
		set(ISPC_SRC "${EXPERIMENT_DIR}/${EXPERIMENT_NAME}.ispc")
		set(ISPC_ASM_NAME "${EXPERIMENT_NAME}_ispc.s")
		set(ISPC_HEADER_NAME "${CMAKE_CURRENT_BINARY_DIR}/${EXPERIMENT_NAME}_ispc.h")

		# assuming the ispc files are independent and don't require any definition
		add_custom_command(
			OUTPUT  ${ISPC_ASM_NAME}
			DEPENDS ${ISPC_SRC}
			COMMAND ${ISPC_EXECUTABLE} ${ISPC_SRC} -O3 --emit-asm -h ${ISPC_HEADER_NAME} -o ${ISPC_ASM_NAME}
		)

		target_sources(${TEST_EXE_NAME} PRIVATE ${ISPC_ASM_NAME})
		target_include_directories(${TEST_EXE_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

		target_sources(${BENCH_EXE_NAME} PRIVATE ${ISPC_ASM_NAME})
		target_include_directories(${BENCH_EXE_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
	endif()

	add_test(NAME ${TEST_EXE_NAME} COMMAND ${TEST_EXE_NAME})
endforeach()
