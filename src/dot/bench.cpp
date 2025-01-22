#include <benchmark/benchmark.h>
#include "dot.hpp"


// the maximum buffer size should be a parameter selectable from CMake, but also queriable from CMake scripts
#ifndef BUFSIZE
	#define BUFSIZE (1 << 28)
#endif


#ifdef USE_FLOAT32
	using floating = float;
#else
	using floating = double;
#endif


constexpr size_t NELEMENTS = (BUFSIZE / 2) / sizeof(floating);


#ifdef USE_STATIC_MEMORY
	#include <array>
	std::array<floating, NELEMENTS> x;
	std::array<floating, NELEMENTS> y;
#else
	#include <vector>
	std::vector<floating> x(NELEMENTS);
	std::vector<floating> y(NELEMENTS);
#endif


template <floating (*impl)(const size_t, const floating*, const floating*)>
static void BM_dot(benchmark::State &state) {
	const int total_bytes_processed = state.range(0);
	const int n = (total_bytes_processed / 2) / sizeof(floating);

	for (auto _ : state) {
		auto result = impl(n, x.data(), y.data());
		benchmark::DoNotOptimize(result);
	}
}


BENCHMARK_TEMPLATE(BM_dot, dot_naive                            )->RangeMultiplier(2)->Range(1024, BUFSIZE);
BENCHMARK_TEMPLATE(BM_dot, dot_templated_unrolling<floating, 2 >)->RangeMultiplier(2)->Range(1024, BUFSIZE);
BENCHMARK_TEMPLATE(BM_dot, dot_templated_unrolling<floating, 4 >)->RangeMultiplier(2)->Range(1024, BUFSIZE);
BENCHMARK_TEMPLATE(BM_dot, dot_templated_unrolling<floating, 8 >)->RangeMultiplier(2)->Range(1024, BUFSIZE);
BENCHMARK_TEMPLATE(BM_dot, dot_templated_unrolling<floating, 16>)->RangeMultiplier(2)->Range(1024, BUFSIZE);

#ifdef EXPERIMENTAL_SIMD_SUPPORT
BENCHMARK_TEMPLATE(BM_dot, dot_experimental_simd<floating>      )->RangeMultiplier(2)->Range(1024, BUFSIZE);
#endif

#ifdef ISPC_SUPPORT
BENCHMARK_TEMPLATE(BM_dot, dot_ispc<floating>                   )->RangeMultiplier(2)->Range(1024, BUFSIZE);
#endif

BENCHMARK_MAIN();
