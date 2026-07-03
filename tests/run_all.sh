#!/usr/bin/env bash
set -euo pipefail

# Regression runner for automatic integration tests.
#
# Network/manual tests are intentionally skipped unless RUN_NETWORK_TESTS=1 is
# set, because they require a second machine or VM and human confirmation.

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TEST_DIR="$PROJECT_ROOT/tests/integration"

tests=(
    "$TEST_DIR/test_01_uapi.sh"
    "$TEST_DIR/test_02_kernel_build_load.sh"
    "$TEST_DIR/test_03_device_ioctl.sh"
    "$TEST_DIR/test_04_cli_basic.sh"
    "$TEST_DIR/test_05_rules_table.sh"
    "$TEST_DIR/test_08_debugfs.sh"
)

if [[ "${RUN_NETWORK_TESTS:-0}" == "1" ]]; then
    for network_test in \
        "$TEST_DIR/test_06_netfilter_log_only.sh" \
        "$TEST_DIR/test_07_firewall_drop.sh"
    do
        [[ -f "$network_test" ]] && tests+=("$network_test")
    done
fi

passed=0
failed=0

for test_script in "${tests[@]}"; do
    echo
    echo "==> Running ${test_script#$PROJECT_ROOT/}"

    if bash "$test_script"; then
        passed=$((passed + 1))
    else
        failed=$((failed + 1))
    fi
done

echo
echo "Summary:"
echo "  passed: $passed"
echo "  failed: $failed"

if [[ "$failed" -ne 0 ]]; then
    exit 1
fi

echo "[PASS] automatic regression tests passed"
