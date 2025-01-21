#include <span>
#include <array>
#include <vector>
#include <benchmark/benchmark.h>
#include "dot.hpp"


// TODO: this may be queried at compile time from CMake, ideally I want 512 MB
#ifndef BUFSIZE
#define BUFSIZE (1 << 28)
#endif


constexpr size_t NELEMENTS = (BUFSIZE / 2) / sizeof(double);


#ifdef USE_STATIC_MEMORY
std::array<double, NELEMENTS> x;
std::array<double, NELEMENTS> y;

#else
std::vector<double> x(NELEMENTS);
std::vector<double> y(NELEMENTS);

#endif


// TODO: it's very ugly to template on a function pointer, can we do better?
template <double (*impl)(const std::span<double>&, const std::span<double>&)>
static void BM_dot(benchmark::State &state) {
	const int total_bytes_processed = state.range(0);
	const int n = (total_bytes_processed / 2) / sizeof(double);

	std::vector<double> first(x.begin(), x.begin() + n);
	std::vector<double> second(y.begin(), y.begin() + n);

	for (auto _ : state) {
		auto result = impl(
			std::span(first),
			std::span(second)
		);
		benchmark::DoNotOptimize(result);
	}
}


BENCHMARK_TEMPLATE(BM_dot, dot_naive                          )->RangeMultiplier(2)->Range(1024, BUFSIZE);
BENCHMARK_TEMPLATE(BM_dot, dot_templated_unrolling<double, 2 >)->RangeMultiplier(2)->Range(1024, BUFSIZE);
BENCHMARK_TEMPLATE(BM_dot, dot_templated_unrolling<double, 4 >)->RangeMultiplier(2)->Range(1024, BUFSIZE);
BENCHMARK_TEMPLATE(BM_dot, dot_templated_unrolling<double, 8 >)->RangeMultiplier(2)->Range(1024, BUFSIZE);
BENCHMARK_TEMPLATE(BM_dot, dot_templated_unrolling<double, 16>)->RangeMultiplier(2)->Range(1024, BUFSIZE);

#ifdef EXPERIMENTAL_SIMD_SUPPORT
BENCHMARK_TEMPLATE(BM_dot, dot_experimental_simd<double>      )->RangeMultiplier(2)->Range(1024, BUFSIZE);
#endif

#ifdef ISPC_SUPPORT
BENCHMARK_TEMPLATE(BM_dot, dot_ispc<double>                   )->RangeMultiplier(2)->Range(1024, BUFSIZE);
#endif

BENCHMARK_MAIN();
