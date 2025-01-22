#include <benchmark/benchmark.h>
#include "minimum.hpp"


// the maximum buffer size should be a parameter selectable from CMake, but also queriable from CMake scripts
#ifndef BUFSIZE
	#define BUFSIZE (1 << 28)
#endif


// I'm using `define` and not `using` this way the benchmark names will have `float` and `double` and not just `floating`
#ifdef USE_FLOAT32
	#define floating float
	#define floating_typestr "float"
#else
	#define floating double
	#define floating_typestr "double"
#endif


constexpr size_t NELEMENTS = BUFSIZE / sizeof(floating);


#ifdef USE_STATIC_MEMORY
	#include <array>
	std::array<floating, NELEMENTS> x;
	std::array<floating, NELEMENTS> y;

	#define memory_type "static memory"
#else
	#include <vector>
	std::vector<floating> x(NELEMENTS);
	std::vector<floating> y(NELEMENTS);

	#define memory_type "heap memory"
#endif


template <floating (*impl)(const size_t, const floating *)>
static void BM_minimum(benchmark::State &state) {
	const auto total_bytes_processed = state.range(0);
	const auto n = total_bytes_processed / sizeof(floating);

	for (auto _ : state) {
		auto result = impl(n, x.data());
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


// this main has been copied from the macro expansion of `BENCHMARK_MAIN`
// needed to complete the report with the floating point type used
int main(int argc, char** argv) {
	char arg0_default[] = "benchmark";
	char* args_default = arg0_default;

	if (!argv) {
		argc = 1;
		argv = &args_default;
	}

	benchmark::Initialize(&argc, argv);

	if (benchmark::ReportUnrecognizedArguments(argc, argv)){
		return 1;
	}

	benchmark::AddCustomContext("experiment", "dot product");
	benchmark::AddCustomContext("floating_type", floating_typestr);
	benchmark::AddCustomContext("memory_type", memory_type);
	benchmark::RunSpecifiedBenchmarks();
	benchmark::Shutdown();

	return 0;
}
