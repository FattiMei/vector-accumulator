#include <benchmark/benchmark.h>
#include "minimum.hpp"


// the maximum buffer size should be a parameter selectable from CMake, but also queriable from CMake scripts
#ifndef BUFSIZE
	#define BUFSIZE (1 << 28)
#endif


#ifdef USE_FLOAT32
	using floating = float;
#else
	using floating = double;
#endif


constexpr size_t NELEMENTS = BUFSIZE / sizeof(floating);


#ifdef USE_STATIC_MEMORY
	#include <array>
	std::array<floating, NELEMENTS> x;
#else
	#include <vector>
	std::vector<floating> x(NELEMENTS);
#endif


template <floating (*impl)(const size_t, const floating *)>
static void BM_minimum(benchmark::State &state) {
	const int total_bytes_processed = state.range(0);
	const int n = total_bytes_processed / sizeof(floating);

	for (auto _ : state) {
		auto result = impl(x.size(), x.data());
		benchmark::DoNotOptimize(result);
	}
}


BENCHMARK_TEMPLATE(BM_minimum, minimum_naive                            )->RangeMultiplier(2)->Range(1024, BUFSIZE);
BENCHMARK_TEMPLATE(BM_minimum, minimum_templated_unrolling<floating, 2 >)->RangeMultiplier(2)->Range(1024, BUFSIZE);
BENCHMARK_TEMPLATE(BM_minimum, minimum_templated_unrolling<floating, 4 >)->RangeMultiplier(2)->Range(1024, BUFSIZE);
BENCHMARK_TEMPLATE(BM_minimum, minimum_templated_unrolling<floating, 8 >)->RangeMultiplier(2)->Range(1024, BUFSIZE);
BENCHMARK_TEMPLATE(BM_minimum, minimum_templated_unrolling<floating, 16>)->RangeMultiplier(2)->Range(1024, BUFSIZE);

#ifdef EXPERIMENTAL_SIMD_SUPPORT
BENCHMARK_TEMPLATE(BM_minimum, minimum_experimental_simd<floating>      )->RangeMultiplier(2)->Range(1024, BUFSIZE);
#endif

#ifdef ISPC_SUPPORT
BENCHMARK_TEMPLATE(BM_minimum, minimum_ispc<floating>                   )->RangeMultiplier(2)->Range(1024, BUFSIZE);
#endif

BENCHMARK_MAIN();
