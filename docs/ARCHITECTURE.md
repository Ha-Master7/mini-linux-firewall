# Architecture

## Purpose

This repository is organized as a small Linux firewall project with two main subsystems:
- `kernel/` for the kernel module implementation.
- `user/` for the user-space CLI client.

A shared interface is defined in `include/`, and helper files, scripts, and test placeholders live in their own directories.

## Machine-friendly structure

```
mini-linux-firewall/
├── include/
│   └── mfw_uapi.h
│
├── kernel/
│   ├── include/
│   │   ├── mfw_device.h
│   │   ├── mfw_rules.h
│   │   ├── mfw_packet.h
│   │   ├── mfw_engine.h
│   │   ├── mfw_netfilter.h
│   │   └── mfw_debugfs.h
│   │
│   ├── src/
│   │   ├── mfw_main.c
│   │   ├── mfw_device.c
│   │   ├── mfw_rules.c
│   │   ├── mfw_packet.c
│   │   ├── mfw_engine.c
│   │   ├── mfw_netfilter.c
│   │   └── mfw_debugfs.c
│   │
│   └── Makefile
│
├── user/
│   ├── include/
│   │   ├── cli_parser.h
│   │   ├── mfw_client.h
│   │   └── printer.h
│   │
│   ├── src/
│   │   ├── main.c
│   │   ├── cli_parser.c
│   │   ├── mfw_client.c
│   │   └── printer.c
│   │
│   └── Makefile
│
├── docs/
│   ├── ARCHITECTURE.md
│   ├── INTERVIEW_NOTES.md
│   ├── TEST_PLAN.md
│   └── DEVELOPMENT_STAGES.md
│
├── scripts/
│   ├── load_module.sh
│   ├── unload_module.sh
│   ├── run_basic_test.sh
│   └── show_debug.sh
│
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── run_all.sh
│   └── README.md
│
├── Makefile
└── README.md
```

## Human-friendly overview

### Top-level files
- `Makefile`: builds the project and coordinates kernel and user targets.
- `README.md`: high-level project description and usage notes.
- `docs/`: architecture, development plan, test plan, and notes.
- `scripts/`: helper shell scripts for module actions and debugging.
- `tests/`: unit tests, integration tests, and the test runner.

### Shared interface
- `include/mfw_uapi.h`: shared user/kernel API definitions.
  - Defines rule structure, ioctl commands, and constants.
  - Used by both the kernel module and the CLI.

### Kernel subsystem
- `kernel/Makefile`: builds the kernel module.
- `kernel/mfw.c`: main kernel module source.
- `kernel/include/`: module-specific headers.
  - `mfw_device.h`: device interface definitions.
  - `mfw_rules.h`: rule table declarations.
  - `mfw_packet.h`: packet handling structures.
  - `mfw_engine.h`: firewall engine interface.
  - `mfw_netfilter.h`: Netfilter hook declarations.
  - `mfw_debugfs.h`: debugfs support declarations.
- `kernel/src/`: modular source files for the kernel implementation.
  - `mfw_main.c`: module initialization and cleanup.
  - `mfw_device.c`: character device / ioctl handling.
  - `mfw_rules.c`: rule management logic.
  - `mfw_packet.c`: packet inspection and matching.
  - `mfw_engine.c`: packet decision engine.
  - `mfw_netfilter.c`: Netfilter hook registration.
  - `mfw_debugfs.c`: debugfs status output.

### User-space subsystem
- `user/Makefile`: builds the CLI client.
- `user/mfwctl.c`: top-level CLI entry point.
- `user/include/`: user-space helper headers.
  - `cli_parser.h`: command-line parsing interface.
  - `mfw_client.h`: client-side API for talking to the kernel module.
  - `printer.h`: output formatting helpers.
- `user/src/`: user implementation files.
  - `main.c`: application startup and command dispatch.
  - `cli_parser.c`: option parsing logic.
  - `mfw_client.c`: ioctl communication with `/dev/mfw`.
  - `printer.c`: formatted user output.

## Runtime flow

1. User runs `make` at the top level.
2. `kernel/Makefile` builds the kernel module; `user/Makefile` builds the CLI.
3. The CLI communicates with the loaded kernel module through `/dev/mfw` and ioctl commands defined in `include/mfw_uapi.h`.
4. The kernel module filters IPv4 packets through a Netfilter hook and exposes internal state via debugfs under `/sys/kernel/debug/mfw/rules`.

## Notes for extension

- The current structure is a skeleton, so source files are placeholders.
- Real implementation files should populate `kernel/src/` and `user/src/` with the actual logic.
- The split between `include/`, `kernel/`, and `user/` keeps shared API, kernel code, and user code separate and easy to maintain.



User Space:
- CLI
- parsing commands
- sending ioctl
- printing rules

Kernel Space:
- device file
- ioctl handler
- rule table
- Netfilter hook
- packet decision
- counters/logging

Shared:
- mfw_uapi.h
- structs
- ioctl definitions
