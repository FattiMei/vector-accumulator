#include <cmath>
#include <array>
#include <string>
#include <vector>
#include <random>
#include <cassert>
#include <iostream>
#include <functional>
#include "dot.hpp"


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
	using dot_impl = std::function<floating(const size_t, const floating*, const floating*)>;
	std::default_random_engine gen(420);

	auto x = generate_random_sequence<floating>(99999, gen);
	auto y = generate_random_sequence<floating>(99999, gen);
	const auto reference = dot_naive(x.size(), x.data(), y.data());

	const auto recipe = {
		  std::make_pair("manual_unroll(2)", dot_templated_unrolling<floating, 2>)
		, std::make_pair("manual_unroll(4)", dot_templated_unrolling<floating, 4>)
		, std::make_pair("manual_unroll(8)", dot_templated_unrolling<floating, 8>)

#ifdef EXPERIMENTAL_SIMD_SUPPORT
		, std::make_pair("native_simd     ", dot_experimental_simd<floating>)
#endif

#ifdef ISPC_SUPPORT
		, std::make_pair("ispc            ", dot_ispc<floating>)
#endif
	};

	for (const auto& [name, impl] : recipe) {
		const auto alternative = impl(x.size(), x.data(), y.data());
		const auto rel = std::abs(reference - alternative) / std::abs(reference);

		if (rel > 1e-12) {
			return 1;
		}

		std::cout << name << ' ' << rel << std::endl;
	}

	return 0;
}
