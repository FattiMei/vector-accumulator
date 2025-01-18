# vector-accumulator
Reduction (also known as accumulation) is an algorithm for processing a list of elements. Examples of reductions are taking the sum of all elements or the minimum of a list of numbers.

The reduction operator needs a list of elements of type A and an associative binary operator on A. Using pseudonotation:
```(haskell)
  reduce + (a,b,c,d,...) == (((a + b) + c) + d) + ...
```

This repository presents a technique for accelerating the reductions of primitive types (like float, double, int..) using SIMD.
