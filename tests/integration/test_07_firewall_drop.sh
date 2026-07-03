#!/usr/bin/env bash
set -euo pipefail

# Stage 7 manual integration test: firewall DROP behavior.
#
# Goal:
#   Verify that adding a DROP rule for a client IP blocks traffic, increments
#   the hit counter, and that deleting the rule allows traffic again.
#
# Usage:
#   sudo bash tests/integration/test_07_firewall_drop.sh <CLIENT_IP>

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
KERNEL_DIR="$PROJECT_ROOT/kernel"
USER_DIR="$PROJECT_ROOT/user"
MODULE_PATH="$KERNEL_DIR/mfw.ko"
MFWCTL="$USER_DIR/mfwctl"
CLIENT_IP="${1:-}"

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

[[ -n "$CLIENT_IP" ]] || fail "Usage: sudo bash tests/integration/test_07_firewall_drop.sh <CLIENT_IP>"

trap cleanup EXIT

make -C "$KERNEL_DIR"
make -C "$USER_DIR"

cleanup
sudo insmod "$MODULE_PATH"
sudo "$MFWCTL" clear

echo "Firewall VM IP addresses:"
hostname -I || true
echo
echo "Before DROP: ping this Firewall VM from the Client VM ($CLIENT_IP)."
read -r -p "Press Enter after confirming ping works..."

sudo "$MFWCTL" add "$CLIENT_IP" drop

echo
echo "DROP rule added for $CLIENT_IP."
echo "Ping again from the Client VM. It should fail or show packet loss."
read -r -p "Press Enter after testing blocked ping..."

output="$(sudo "$MFWCTL" list)"
echo "$output"
require_output_contains "$output" "$CLIENT_IP"
require_output_contains "$output" "DROP"

echo "$output" | awk -v ip="$CLIENT_IP" '$0 ~ ip { if ($NF > 0) found=1 } END { exit found ? 0 : 1 }' || \
    fail "Expected HITS to be greater than 0 for $CLIENT_IP"

sudo "$MFWCTL" del "$CLIENT_IP"

echo
echo "DROP rule deleted. Ping from the Client VM again; it should work."
read -r -p "Press Enter after confirming ping works again..."

sudo rmmod mfw
trap - EXIT

echo "[PASS] DROP rule blocks traffic from client IP and counter increases"
