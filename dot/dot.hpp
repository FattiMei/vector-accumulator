#ifndef __DOT_HPP__
#define __DOT_HPP__


#include <array>
#include <vector>
#include <numeric>


template <typename T>
T dot_naive(const std::vector<T>& x, const std::vector<T>& y) {
	T acc = 0;

	for (size_t i = 0; i < x.size(); ++i) {
		acc += x[i] * y[i];
	}

	return acc;
}


template <typename T, size_t UNROLL_FACTOR>
T dot_templated_unrolling(const std::vector<T>& x, const std::vector<T>& y) {
	std::array<T, UNROLL_FACTOR> acc{0};
	size_t i;

	for (i = 0; i + UNROLL_FACTOR <= x.size(); i += UNROLL_FACTOR) {
		for (auto j = 0; j < UNROLL_FACTOR; ++j) {
			acc[j] += x[i+j] * y[i+j];
		}
	}

	// loop peeling
	if (i < x.size()) {
		for (auto j = 0; j < x.size() - i; ++j) {
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
T dot_experimental_simd(const std::vector<T>& x, const std::vector<T>& y) {
	using simd_t = std::experimental::native_simd<T>;
	simd_t acc = 0, vx, vy;
	size_t i;

	for (i = 0; i + acc.size() <= x.size(); i += acc.size()) {
		vx.copy_from(x.data() + i, std::experimental::element_aligned);
		vy.copy_from(y.data() + i, std::experimental::element_aligned);

		acc += vx * vy;
	}

	// loop peeling
	T remainder = 0;
	for (; i < x.size(); ++i) {
		remainder += x[i] * y[i];
	}

	return std::experimental::reduce(acc) + remainder;
}

#endif


#endif
