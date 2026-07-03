#!/usr/bin/env bash
set -euo pipefail

# Stage 5 integration test: kernel rule table.
#
# Goal:
#   Verify real add/list/delete/clear behavior, duplicate rejection, and CLI
#   validation for invalid IP/action input.

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
KERNEL_DIR="$PROJECT_ROOT/kernel"
USER_DIR="$PROJECT_ROOT/user"
MODULE_PATH="$KERNEL_DIR/mfw.ko"
MFWCTL="$USER_DIR/mfwctl"

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

require_command_fails() {
    set +e
    "$@" >/tmp/mfw_test_output.$$ 2>&1
    local rc=$?
    set -e

    if [[ "$rc" -eq 0 ]]; then
        cat /tmp/mfw_test_output.$$
        rm -f /tmp/mfw_test_output.$$
        fail "Expected command to fail: $*"
    fi

    rm -f /tmp/mfw_test_output.$$
}

cleanup() {
    sudo rmmod mfw 2>/dev/null || true
}

trap cleanup EXIT

make -C "$KERNEL_DIR"
make -C "$USER_DIR"

[[ -f "$MODULE_PATH" ]] || fail "Missing file: $MODULE_PATH"
[[ -x "$MFWCTL" ]] || fail "Missing executable: $MFWCTL"

cleanup
sudo insmod "$MODULE_PATH"

sudo "$MFWCTL" clear

output="$(sudo "$MFWCTL" list)"
require_output_contains "$output" "Rule count: 0"

sudo "$MFWCTL" add 192.168.1.5 drop
output="$(sudo "$MFWCTL" list)"
require_output_contains "$output" "Rule count: 1"
require_output_contains "$output" "192.168.1.5"
require_output_contains "$output" "DROP"

require_command_fails sudo "$MFWCTL" add 192.168.1.5 drop

sudo "$MFWCTL" del 192.168.1.5
output="$(sudo "$MFWCTL" list)"
require_output_contains "$output" "Rule count: 0"

sudo "$MFWCTL" add 192.168.1.5 drop
sudo "$MFWCTL" add 10.0.0.8 pass
output="$(sudo "$MFWCTL" list)"
require_output_contains "$output" "Rule count: 2"
require_output_contains "$output" "10.0.0.8"
require_output_contains "$output" "PASS"

sudo "$MFWCTL" clear
output="$(sudo "$MFWCTL" list)"
require_output_contains "$output" "Rule count: 0"

require_command_fails "$MFWCTL" add invalid_ip drop
require_command_fails "$MFWCTL" add 192.168.1.5 invalid_action

sudo rmmod mfw
trap - EXIT

echo "[PASS] rule table add/list/delete/clear works correctly"
