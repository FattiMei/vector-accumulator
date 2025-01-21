#include <span> 
#include <cmath>
#include <array>
#include <string>
#include <vector>
#include <random>
#include <cassert>
#include <iostream>
#include <functional>
#include "minimum.hpp"


template <typename T>
std::vector<T> generate_random_sequence(size_t n, std::default_random_engine &gen) {
	std::vector<T> seq(n);
	std::uniform_real_distribution<T> uniform(-1.0, 1.0);

	for (auto &x : seq) {
		x = uniform(gen);
	}

	return seq;
}


int main() {
	using floating = double;
	using dot_impl = std::function<floating(const std::span<floating>&)>;
	std::default_random_engine gen(420);

	auto x = generate_random_sequence<floating>(99999, gen);
	const auto reference = minimum_naive(std::span(x));

	const auto recipe = {
		  std::make_pair("manual_unroll(2)", minimum_templated_unrolling<floating, 2>)
		, std::make_pair("manual_unroll(4)", minimum_templated_unrolling<floating, 4>)
		, std::make_pair("manual_unroll(8)", minimum_templated_unrolling<floating, 8>)

#ifdef EXPERIMENTAL_SIMD_SUPPORT
		, std::make_pair("native_simd     ", minimum_experimental_simd<floating>)
#endif

#ifdef ISPC_SUPPORT
		, std::make_pair("ispc            ", minimum_ispc<floating>)
#endif
	};

	for (const auto& [name, impl] : recipe) {
		const auto alternative = impl(x);

		if (reference != alternative) {
			return 1;
		}

		std::cout << name << ' ' << alternative << std::endl;
	}

	return 0;
}
