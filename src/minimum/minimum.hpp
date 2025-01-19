#ifndef __MINIMUM_HPP__
#define __MINIMUM_HPP__


#include <array>
#include <limits>
#include <vector>
#include <numeric>
#include <algorithm>


template <typename T>
T minimum_naive(const std::vector<T>& x) {
	T min = std::numeric_limits<T>::max();

	for (size_t i = 0; i < x.size(); ++i) {
		min = std::min(min, x[i]);
	}

	return min;
}


template <typename T, size_t UNROLL_FACTOR>
T minimum_templated_unrolling(const std::vector<T>& x) {
	std::array<T, UNROLL_FACTOR> acc{std::numeric_limits<T>::max()};
	size_t i;

	for (i = 0; i + UNROLL_FACTOR <= x.size(); i += UNROLL_FACTOR) {
		for (auto j = 0; j < UNROLL_FACTOR; ++j) {
			acc[j] = std::min(acc[j], x[i+j]);
		}
	}

	// loop peeling
	if (i < x.size()) {
		for (auto j = 0; j < UNROLL_FACTOR; ++j) {
			acc[j] = std::min(acc[j], x[i+j]);
		}
	}

	return *std::min_element(
		acc.begin(),
		acc.end()
	);
}


#ifdef EXPERIMENTAL_SIMD_SUPPORT

#include <experimental/simd>

template <typename T>
T minimum_experimental_simd(const std::vector<T>& x) {
	using simd_t = std::experimental::native_simd<T>;
	using mask_t = std::experimental::native_simd_mask<T>;

	simd_t acc = std::numeric_limits<T>::max();
	simd_t vx;

	for (size_t i = 0; i + acc.size() <= x.size(); i += acc.size()) {
		vx.copy_from(x.data() + i, std::experimental::element_aligned);

		acc = std::experimental::min(acc, vx);
	}

	return std::experimental::hmin(acc);
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
#include "minimum_ispc.h"
#include <type_traits>


template <typename T>
T minimum_ispc(const std::vector<T>& x) {
	if constexpr (std::is_same_v<T, float>) {
		return ispc::minimumf(x.data(), x.size());
	}
	else if constexpr (std::is_same_v<T, double>) {
		return ispc::minimumd(x.data(), x.size());
	}
}


#endif


#endif
