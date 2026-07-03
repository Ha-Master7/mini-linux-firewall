# Test Instructions

Run these tests from a Linux VM or WSL environment that supports kernel module
building/loading. Kernel and integration tests require `sudo`, `gcc`, `make`,
kernel headers for the running kernel, and access to `dmesg`.

Do not run the kernel-module tests directly from Windows PowerShell.

## Directory Layout

```text
tests/
├── unit/          C unit tests and small kernel stubs
├── integration/   bash integration/manual-stage tests
├── run_all.sh     automatic test runner
└── README.md      this file
```

There are no separate `user_tests/` or `manual_tests/` directories. User-space
logic tests live in `unit/`, and manual network checks live in `integration/`
next to the stage they validate.

## Run All Automatic Tests

From the project root:

```bash
bash tests/run_all.sh
```

`tests/run_all.sh` exports the shared include flags through
`MFW_TEST_CPPFLAGS`, so unit tests can include project headers normally:

```text
-Iinclude -Iuser/include -Ikernel/include
```

This runs:

```text
tests/integration/test_01_uapi.sh
tests/integration/test_02_kernel_build_load.sh
tests/integration/test_03_device_ioctl.sh
tests/integration/test_04_cli_basic.sh
tests/integration/test_05_rules_table.sh
tests/integration/test_08_debugfs.sh
```

Manual network tests are skipped by default.

## Run Unit Tests Only

```bash
bash tests/integration/test_01_uapi.sh
```

This compiles and runs the unit tests under `tests/unit/`:

```text
uapi_test.c
cli_main_logic_test.c
mfw_client_test.c
mfw_rules_test.c
skeleton_contract_test.c
```

## Run Individual Integration Tests

```bash
bash tests/integration/test_02_kernel_build_load.sh
bash tests/integration/test_03_device_ioctl.sh
bash tests/integration/test_04_cli_basic.sh
bash tests/integration/test_05_rules_table.sh
bash tests/integration/test_08_debugfs.sh
```

## Run Manual Network Tests

These tests need another VM or host that can send traffic to the firewall VM.

```bash
RUN_NETWORK_TESTS=1 bash tests/run_all.sh
```

Or run them one by one:

```bash
bash tests/integration/test_06_netfilter_log_only.sh
sudo bash tests/integration/test_07_firewall_drop.sh <CLIENT_IP>
```

Example:

```bash
sudo bash tests/integration/test_07_firewall_drop.sh 192.168.56.101
```

## Reload Module During Development

```bash
bash scripts/reset_module.sh
```

This unloads `mfw` if it is already loaded, rebuilds the kernel module, loads it
again, prints `/dev/mfw` if it exists, and shows recent `dmesg` output.

## Expected TDD Behavior

Some later-stage tests are expected to fail until their implementation exists.
For example, rule-table, Netfilter, DROP, and debugfs tests require the matching
kernel code to be wired into the module.

Each test prints `[PASS]` on success and exits with a non-zero status on failure.
