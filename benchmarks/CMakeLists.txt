# Hydraulic performance benchmark.
#
# This target is intentionally not registered with CTest. Wall-clock timing is
# useful for local/release comparisons but is too environment-dependent for CI
# pass/fail decisions. Deterministic hydraulic guardrails live in the normal
# test_toolkit suite.

add_executable(benchmark_hydraulics hydraulic_benchmark.cpp)
target_link_libraries(benchmark_hydraulics epanet2)
target_compile_definitions(benchmark_hydraulics PRIVATE
    EPANET_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

set_property(TARGET benchmark_hydraulics PROPERTY CXX_STANDARD 11)
set_property(TARGET benchmark_hydraulics PROPERTY CXX_STANDARD_REQUIRED ON)
