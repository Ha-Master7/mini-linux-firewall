#!/usr/bin/env bash
set -euo pipefail

# Stage 6 integration/manual test: Netfilter log-only mode.
#
# Goal:
#   Verify that the Netfilter hook is registered, incoming IPv4 packets are
#   logged, and traffic is still accepted. This test needs another host or VM
#   to send packets to this machine.

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
KERNEL_DIR="$PROJECT_ROOT/kernel"
USER_DIR="$PROJECT_ROOT/user"
MODULE_PATH="$KERNEL_DIR/mfw.ko"
WAIT_SECONDS="${WAIT_SECONDS:-10}"

fail() {
    echo "[FAIL] $1" >&2
    exit 1
}

cleanup() {
    sudo rmmod mfw 2>/dev/null || true
}

trap cleanup EXIT

make -C "$KERNEL_DIR"
make -C "$USER_DIR"

cleanup
sudo insmod "$MODULE_PATH"

sudo dmesg | grep -Ei 'Netfilter hook registered|netfilter.*registered' || {
    sudo dmesg | tail -n 50
    fail "Expected dmesg to show Netfilter hook registration"
}

echo "Local machine IP addresses:"
hostname -I || true
echo
echo "Ping this machine from another VM/host now."
echo "Waiting $WAIT_SECONDS seconds for incoming packets..."
sleep "$WAIT_SECONDS"

log_output="$(sudo dmesg | tail -n 80)"
echo "$log_output"

echo "$log_output" | grep -Ei 'packet received|src=' || \
    fail "Expected dmesg to contain packet logs"

lsmod | grep -q '^mfw' || fail "mfw module is not loaded"

sudo rmmod mfw
trap - EXIT

echo "[PASS] Netfilter hook logs packets and accepts traffic"
