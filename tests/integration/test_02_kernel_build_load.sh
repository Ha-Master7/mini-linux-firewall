#!/usr/bin/env bash
set -euo pipefail

# Stage 2 integration test: kernel module skeleton.
#
# Goal:
#   Verify that the kernel module can be built, loaded into the running kernel,
#   observed through lsmod/dmesg, and unloaded cleanly.
#
# This is a TDD-style integration test for kernel code. It does not inspect
# firewall behavior yet; it only proves the module lifecycle works.

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
KERNEL_DIR="$PROJECT_ROOT/kernel"
MODULE_PATH="$KERNEL_DIR/mfw.ko"

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

    sudo dmesg | grep -Eq "$expected" || {
        sudo dmesg | tail -n 50
        fail "Expected dmesg to contain: $expected"
    }
}

trap cleanup EXIT

cd "$KERNEL_DIR"

make clean
make

require_file "$MODULE_PATH"

cleanup

sudo insmod "$MODULE_PATH"

lsmod | grep -q '^mfw' || fail "mfw module is not visible in lsmod"
require_dmesg_contains 'Mini Firewall kernel module (loaded|loaded successfully)'

sudo rmmod mfw

require_dmesg_contains 'Mini Firewall kernel module unloaded'

trap - EXIT

pass "kernel module builds, loads and unloads successfully"
