#ifndef __DOT_HPP__
#define __DOT_HPP__


#include <array>
#include <numeric>


template <typename T>
T dot_naive(const size_t n, const T x[], const T y[]) {
	T acc = 0;

	for (size_t i = 0; i < n; ++i) {
		acc += x[i] * y[i];
	}

	return acc;
}


template <typename T, size_t UNROLL_FACTOR>
T dot_templated_unrolling(const size_t n, const T x[], const T y[]) {
	std::array<T, UNROLL_FACTOR> acc{0};
	size_t i;

	for (i = 0; i + UNROLL_FACTOR <= n; i += UNROLL_FACTOR) {
		for (auto j = 0; j < UNROLL_FACTOR; ++j) {
			acc[j] += x[i+j] * y[i+j];
		}
	}

	// loop peeling
	if (i < n) {
		for (auto j = 0; j < n - i; ++j) {
			acc[j] += x[i+j] * y[i+j];
		}
	}

	return std::accumulate(
		acc.begin(),
		acc.end(),
		0.0
	);
}


#ifdef EXPERIMENTAL_SIMD_SUPPORT

#include <experimental/simd>

template <typename T>
T dot_experimental_simd(const size_t n, const T x[], const T y[]) {
	using simd_t = std::experimental::native_simd<T>;
	simd_t acc = 0, vx, vy;
	size_t i;

	for (i = 0; i + acc.size() <= n; i += acc.size()) {
		vx.copy_from(x + i, std::experimental::element_aligned);
		vy.copy_from(y + i, std::experimental::element_aligned);

		acc += vx * vy;
	}

	// loop peeling
	T remainder = 0;
	for (; i < n; ++i) {
		remainder += x[i] * y[i];
	}

	return std::experimental::reduce(acc) + remainder;
}

#endif


#ifdef ISPC_SUPPORT
// this is a hack, originated from my CMake skill issue:
// 	the ISPC compiler produces automatically the header file to be included in the main C++ application,
//
// 	the header name is generated from the ispc source name
// 	  foo.ispc -> foo_ispc.h
//
//	when in doubt grep for
#include "dot_ispc.h"
#include <type_traits>


template <typename T>
T dot_ispc(const size_t n, const T x[], const T y[]) {
	if constexpr (std::is_same_v<T, float>) {
		return ispc::dotf(x, y, n);
	}
	else if constexpr (std::is_same_v<T, double>) {
		return ispc::dotd(x, y, n);
	}
}


#endif


#endif
