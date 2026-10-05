# Hydraulic performance benchmark

`benchmark_hydraulics` is an opt-in benchmark for hydraulic solver work. It is
kept separate from CTest on purpose: wall-clock performance depends on the
machine, compiler, build flags, system load, and power-management state, so a
timing threshold is not a reliable CI gate.

## Build and run

```sh
cmake -S . -B build-perf \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=OFF \
  -DBUILD_BENCHMARKS=ON
cmake --build build-perf --target benchmark_hydraulics -j
./build-perf/bin/benchmark_hydraulics --repetitions 100 --warmup 5
```

With no input paths the benchmark runs the three EPANET example networks plus
`benchmarks/data/Grid20.inp`, a synthetic 20 x 20 grid (401 nodes and 761
links). Custom INP files can be supplied as positional arguments.

Every repetition uses `EN_INITFLOW`, so it starts from EPANET's normal
re-initialized link-flow guess rather than inheriting a solved flow field from
the previous repetition. Model loading and result-signature collection are
outside the timed region.

The CSV output records:

- network size, repetitions, warmup count and hydraulic event count;
- total and maximum GGA iteration counts;
- final hydraulic time;
- maximum relative error, head error and flow change seen during the run;
- final head/flow sums and index-weighted sums as compact numerical signatures;
- total elapsed milliseconds and microseconds per complete hydraulic run.

For branch comparisons, use the same compiler, build type, benchmark source,
repetition count and machine for both branches. The timing columns are
informational; the deterministic result and iteration guardrails are in
`test_hydraulic_performance_guardrails.cpp` and are part of `test_toolkit`.

The CI guardrails intentionally allow iteration counts to decrease, but fail if
Net1, Net2, Net3 or Grid20 require more iterations than this baseline or if
their final hydraulic signatures change outside tight floating-point
tolerances.

## Comparing against `dev`

For final branch characterization, compare Release builds from separate
worktrees on the same machine. Run several interleaved samples; CPU affinity can
be useful on noisy development machines. Do not turn the observed wall-clock
ratio into a CTest threshold.

A useful comparison records both timing and the deterministic columns in the
CSV. Event counts, iteration counts, final time, convergence diagnostics, and
head/flow signatures distinguish a real solver-behavior change from ordinary
timing noise. The CI guardrail test remains the authority for deterministic
behavior, while this benchmark is the authority for local performance
characterization.

When profiling the unit-independent branch, include `EN_initH()` separately
from `EN_runH()`. Compiling `SolverModel` is initialization-boundary work, while
`loadhydraulicsolverstate()` / `savehydraulicsolverstate()` are per-hydraulic-
event boundary work. Keeping those categories separate avoids attributing model
compilation overhead to the iterative GGA itself.
