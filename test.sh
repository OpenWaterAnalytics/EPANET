#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build-tests"
MODE="${1:-all}"

case "$MODE" in
    all|toolkit|hydraulic) ;;
    *)
        echo "Usage: ./test.sh [all|toolkit|hydraulic]" >&2
        exit 2
        ;;
esac

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
    -DBUILD_TESTS=ON \
    -DCMAKE_BUILD_TYPE=Debug

case "$MODE" in
    all)
        cmake --build "$BUILD_DIR" --parallel
        ctest --test-dir "$BUILD_DIR" --output-on-failure
        ;;
    toolkit)
        cmake --build "$BUILD_DIR" --target test_toolkit --parallel
        ctest --test-dir "$BUILD_DIR" -R '^test_toolkit$' --output-on-failure
        ;;
    hydraulic)
        cmake --build "$BUILD_DIR" --target test_toolkit --parallel
        (
            cd "$ROOT_DIR/tests/data"
            "$BUILD_DIR/bin/test_toolkit" \
                --run_test=test_hydraulic_characterization
            "$BUILD_DIR/bin/test_toolkit" \
                --run_test=test_hydraulic_unit_equivalence
        )
        ;;
esac
