#!/usr/bin/env bash
set -euo pipefail

# Stage 1/unit test runner: user-space unit tests.
#
# Goal:
#   Compile and run the user-space unit tests. These cover the shared UAPI,
#   CLI parsing logic, mfw_client ioctl wrapper behavior, kernel rule-table
#   logic with test stubs, and skeleton header contracts.

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
UNIT_DIR="$PROJECT_ROOT/tests/unit"
KERNEL_STUBS="$UNIT_DIR/kernel_stubs"

fail() {
    echo "[FAIL] $1" >&2
    exit 1
}

compile_and_run() {
    local name="$1"
    local src="$2"
    shift 2
    local bin="$UNIT_DIR/$name"

    [[ -f "$src" ]] || fail "Missing file: $src"

    gcc -Wall -Wextra "$@" -o "$bin" "$src"
    "$bin"
}

compile_and_run uapi_test "$UNIT_DIR/uapi_test.c" \
    -I"$PROJECT_ROOT/include"

compile_and_run cli_main_logic_test "$UNIT_DIR/cli_main_logic_test.c" \
    -I"$PROJECT_ROOT/include" \
    -I"$PROJECT_ROOT/user/include"

compile_and_run mfw_client_test "$UNIT_DIR/mfw_client_test.c" \
    -I"$PROJECT_ROOT/include" \
    -I"$PROJECT_ROOT/user/include"

compile_and_run mfw_rules_test "$UNIT_DIR/mfw_rules_test.c" \
    -I"$KERNEL_STUBS" \
    -I"$PROJECT_ROOT/include" \
    -I"$PROJECT_ROOT/kernel/include"

compile_and_run skeleton_contract_test "$UNIT_DIR/skeleton_contract_test.c" \
    -I"$PROJECT_ROOT/user/include" \
    -I"$PROJECT_ROOT/kernel/include"

echo "[PASS] all unit tests passed"
