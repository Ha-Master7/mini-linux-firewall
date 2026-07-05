#!/usr/bin/env bash
set -euo pipefail

# Stage 8 integration test: debugfs rules output.
#
# Goal:
#   Verify that /sys/kernel/debug/mfw/rules exists and reflects rule-table
#   changes after add and clear operations.

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
KERNEL_DIR="$PROJECT_ROOT/kernel"
USER_DIR="$PROJECT_ROOT/user"
MODULE_PATH="$KERNEL_DIR/mfw.ko"
MFWCTL="$USER_DIR/mfwctl"
DEBUG_RULES="/sys/kernel/debug/mfw/rules"

fail() {
    echo "[FAIL] $1" >&2
    exit 1
}

require_output_contains() {
    local output="$1"
    local expected="$2"

    echo "$output" | grep -q "$expected" || {
        echo "$output"
        fail "Expected output to contain: $expected"
    }
}

cleanup() {
    sudo rmmod mfw 2>/dev/null || true
}

trap cleanup EXIT

make -C "$KERNEL_DIR" clean
make -C "$KERNEL_DIR"
make -C "$USER_DIR"

sudo mount -t debugfs none /sys/kernel/debug 2>/dev/null || true

cleanup
sudo insmod "$MODULE_PATH"

sudo dmesg | tail -n 80 | grep -q "debugfs created" || {
    sudo dmesg | tail -n 80
    fail "debugfs init message was not found in dmesg"
}

sudo test -d /sys/kernel/debug/mfw || {
    sudo dmesg | tail -n 80
    sudo ls -la /sys/kernel/debug || true
    fail "Missing debugfs directory: /sys/kernel/debug/mfw"
}

sudo test -f "$DEBUG_RULES" || {
    sudo ls -la /sys/kernel/debug/mfw || true
    fail "Missing debugfs rules file: $DEBUG_RULES"
}

sudo "$MFWCTL" clear
sudo "$MFWCTL" add 192.168.1.5 drop

output="$(sudo cat "$DEBUG_RULES")"
require_output_contains "$output" "192.168.1.5"
require_output_contains "$output" "DROP"

sudo "$MFWCTL" clear

output="$(sudo cat "$DEBUG_RULES")"
if ! echo "$output" | grep -Eq 'Rule count: 0|No rules|0 rules'; then
    echo "$output"
    fail "Expected debugfs rules output to show zero rules after clear"
fi

sudo rmmod mfw
trap - EXIT

echo "[PASS] debugfs rules output updates after add and clear"
