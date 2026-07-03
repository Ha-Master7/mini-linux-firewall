#!/usr/bin/env bash
set -euo pipefail

# Stage 4 integration test: user CLI basic ioctl communication.
#
# Goal:
#   Verify that user/mfwctl can open /dev/mfw and send ioctl commands to the
#   kernel. Rule storage is not required for this stage.

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

[[ -e /dev/mfw ]] || fail "/dev/mfw was not created"

list_output="$(sudo "$MFWCTL" list)"
if ! echo "$list_output" | grep -Eq 'Rule count: 0|No rules found'; then
    echo "$list_output"
    fail "Expected list output to show an empty rule list"
fi

clear_output="$(sudo "$MFWCTL" clear)"
require_output_contains "$clear_output" "Rules cleared"

set +e
add_output="$(sudo "$MFWCTL" add 192.168.1.5 drop 2>&1)"
add_rc=$?
set -e

if [[ "$add_rc" -ne 0 && "$add_rc" -ne 2 ]]; then
    echo "$add_output"
    fail "mfwctl add failed with unexpected exit code: $add_rc"
fi

if [[ "$add_rc" -eq 2 ]]; then
    require_output_contains "$add_output" "Rule Table is not implemented yet"
fi

sudo dmesg | grep -q "ADD_RULE received" || {
    sudo dmesg | tail -n 50
    fail "Expected dmesg to show ADD_RULE received"
}

sudo rmmod mfw
trap - EXIT

echo "[PASS] CLI can communicate with kernel through ioctl"
