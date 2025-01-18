# vector-accumulator
This repository implements SIMD techniques in C++ for accelerating reductions on primitive types (float, double, int, ...)


## Context
I'm building on:
  * [SIMD intrinsics: A Benchmark Study](https://ishuah.com/2021/12/19/a-benchmark-study-simd-intrinsics/)
  * [An introduction to SIMD and ISPC in Rust](https://state.smerity.com/smerity/state/01E8RNH7HRRJT2A63NSX3N6SP1)

We are trying to accelerate the computation of reductions (sum of numbers, dot product between vectors) using some parallel processing. There exist concurrency in the reduction algorithm because two or more threads can partition the set and independently accumulate partial results. At the end the local accumulators are once again accumulated into the final result.


## Requirements and benefits of SIMD
When the input data is *stored in a contiguous chunk of memory* and properly aligned, the (logical) threads can be grouped into a single instruction. When applicable, SIMD is a more efficient paradigm than multithreading because of:
  * low overhead
  * automatic barriers
  * efficient use of transistors (spent on actual ALUs rather then on control logic)


## Pitfall when estimating program performance
If the target architecture has support for 4-lane SIMD registers does it implies a 4x speedup? NO.
>"Anyone can build a fast CPU. The trick is to build a fast system." - Seymour Cray

It doesn't matter the speed at which SIMD instructions can be performed if the system can't provide the ALUs with fresh data at the same pace. One typically finds the memory to be the main bottleneck in those computations. A throughput oriented mindset unlocks new optimizations like:
  * kernel fusion (omits memory transactions because partial results stay in registers)
  * hiding latency (overlaps memory reads with other useful computation)


# C++ techniques
Implementing SIMD in C++ is a matter of:
  * hardware support: fortunately as january 2025 most cpus out there supports SIMD
  * portability: writing x86 intrinsics is not portable to ARM devices

The reference articles implements SIMD with hand written x86 intrinsics and the `ispc` intel compiler. I complete this list with:
  * [C++ experimental SIMD library](https://en.cppreference.com/w/cpp/experimental/simd)
  * metaprogramming assisted loop unrolling

A comparison of the program runtimes and generated code will be included in the repository.
