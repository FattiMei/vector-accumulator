cmake_minimum_required(VERSION 3.20)
project(vector-accumulator)


# language configuration
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 23)


# experiment enumeration
set(DOT_PRODUCT_DIR "${CMAKE_SOURCE_DIR}/dot")


add_executable(dot_product_test ${DOT_PRODUCT_DIR}/test.cpp)
