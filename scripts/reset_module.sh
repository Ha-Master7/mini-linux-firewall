#!/usr/bin/env bash
set -euo pipefail

# Rebuild and reload the mfw kernel module from a clean state.
# Useful while iterating on kernel code between integration-test runs.

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
KERNEL_DIR="$PROJECT_ROOT/kernel"

cd "$KERNEL_DIR"

sudo rmmod mfw 2>/dev/null || true
make clean
make
sudo insmod mfw.ko

echo "[OK] mfw module reloaded"
ls -l /dev/mfw || true
sudo dmesg | tail -n 20
