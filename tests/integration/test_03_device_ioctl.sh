#!/usr/bin/env bash
set -euo pipefail

# Stage 3 integration test: kernel device layer.
#
# Goal:
#   Verify that loading the module creates /dev/mfw as a character device and
#   that the device registration path reports success in dmesg.
#
# This test intentionally does not validate rule storage. That belongs to the
# later rule-table stage.

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
KERNEL_DIR="$PROJECT_ROOT/kernel"
MODULE_PATH="$KERNEL_DIR/mfw.ko"
DEVICE_PATH="/dev/mfw"

pass() {
    echo "[PASS] $1"
}

fail() {
    echo "[FAIL] $1" >&2
    exit 1
}

cleanup() {
    sudo rmmod mfw 2>/dev/null || true
}

require_file() {
    local path="$1"

    [[ -e "$path" ]] || fail "Missing file: $path"
}

require_dmesg_contains() {
    local expected="$1"
    local dmesg_output

    dmesg_output="$(sudo dmesg)"

    grep -q "$expected" <<< "$dmesg_output" || {
        tail -n 50 <<< "$dmesg_output"
        fail "Expected dmesg to contain: $expected"
    }
}

trap cleanup EXIT

make -C "$KERNEL_DIR"
require_file "$MODULE_PATH"

cleanup

sudo insmod "$MODULE_PATH"

[[ -e "$DEVICE_PATH" ]] || fail "$DEVICE_PATH was not created"
[[ -c "$DEVICE_PATH" ]] || fail "$DEVICE_PATH exists but is not a character device"

require_dmesg_contains '/dev/mfw registered successfully'

sudo rmmod mfw

trap - EXIT

pass "/dev/mfw device was created successfully"
