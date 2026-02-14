# Linux Process Information Kernel Module

[![CI](https://img.shields.io/github/actions/workflow/status/navidpadid/process-info-kernel-module/ci.yml?branch=main&style=for-the-badge&logo=github&logoColor=white&label=Build)](https://github.com/navidpadid/process-info-kernel-module/actions/workflows/ci.yml)
[![Last Commit](https://img.shields.io/github/last-commit/navidpadid/process-info-kernel-module?style=for-the-badge&logo=git&logoColor=white)](https://github.com/navidpadid/process-info-kernel-module/commits/main)
[![License](https://img.shields.io/badge/License-Dual%20BSD%2FGPL-blue?style=for-the-badge)](LICENSE)

> A Linux kernel module that extracts detailed process and it's thread information including memory layout, CPU usage, and ELF sections via `/proc` filesystem.

## Features

- **Process Memory Layout**: Code, Data, BSS, Heap, and Stack addresses
- **Memory Pressure Monitoring**: RSS, VSZ, swap usage, page faults (major/minor), and OOM score adjustment
- **Visual Memory Map**: Proportional bar chart visualization of memory regions
- **Open Sockets**: List all open sockets with family (IPv4/IPv6/Unix), type, state, and addresses
- **Network Stats (Brief)**: Per-process TCP counters, socket counts (TCP/UDP/UNIX), drops, and net devices
- **Thread Information**: List all threads with TID, state, CPU usage, priority, and CPU affinity
- **CPU Usage Tracking**: Real-time CPU percentage calculation per process and thread
- **ELF Section Analysis**: Binary base address and section boundaries
- **Proc Interface**: Easy access through `/proc/elf_det/`
- **Comprehensive Testing**: Unit tests and QEMU-based E2E testing
- **Code Quality**: Pre-configured static analysis (sparse, cppcheck, checkpatch)

### Example Output

```
>> Enter process ID (or Ctrl+C to exit): 1219

================================================================================
                          PROCESS INFORMATION                                   
================================================================================
Command line:   ./build/test_multithread 
Process ID:      1219
Name:            test_multithrea
CPU Usage:       2.81%

Memory Pressure Statistics:
--------------------------------------------------------------------------------
  RSS (Resident):  1408 KB
    - Anonymous:   0 KB
    - File-backed: 1408 KB
    - Shared Mem:  0 KB
  VSZ (Virtual):   35464 KB
  Swap Usage:      0 KB
  Page Faults:
    - Major:       0
    - Minor:       119
    - Total:       119
  OOM Score Adj:   0
--------------------------------------------------------------------------------

Memory Layout:
--------------------------------------------------------------------------------
  Code Section:    0x0000603a34f21000 - 0x0000603a34f21459
  Data Section:    0x0000603a34f23d78 - 0x0000603a34f24010
  BSS Section:     0x0000603a34f24010 - 0x0000603a4a0b7000
  Heap:            0x0000603a4a0b7000 - 0x0000603a4a0d8000
  Stack:           0x00007fff7954ba80 - 0x00007fff7952b000
  ELF Base:        0x0000603a34f20000

Memory Layout Visualization:
--------------------------------------------------------------------------------
Low:  0x0000603a34f21000

CODE  (1 KB)
      [=                                                 ]

DATA  (664 B)
      [=                                                 ]

BSS   (337 MB)
      [================================================= ]

HEAP  (132 KB)
      [=                                                 ]

STACK (130 KB)
      [=                                                 ]

High: 0x00007fff7954ba80
--------------------------------------------------------------------------------

[network]
sockets_total: 6 (tcp: 4, udp: 1, unix: 1)
rx_packets: 18342
tx_packets: 12107
rx_bytes: 21458990
tx_bytes: 10822144
tcp_retransmits: 37
drops: 2
net_devices: eth0=4 lo=2

Open Sockets:
--------------------------------------------------------------------------------
  [FD 3] Family: AF_INET     Type: STREAM    State: LISTEN      
          Local:  127.0.0.1:12345  Remote: 0.0.0.0:0
  [FD 4] Family: AF_INET     Type: DGRAM     State: CLOSE       
          Local:  127.0.0.1:12346  Remote: 0.0.0.0:0
  [FD 5] Family: AF_UNIX     Type: STREAM    State: LISTEN      
--------------------------------------------------------------------------------

================================================================================
                          THREAD INFORMATION                                    
================================================================================
TID    NAME             CPU(%)   STATE  PRIORITY  NICE  CPU_AFFINITY
-----  ---------------  -------  -----  --------  ----  ----------------
1219   test_multithrea     2.81   S         0         0  0,1
1221   test_multithrea     0.42   S         0         0  0,1
1222   test_multithrea     0.14   S         0         0  0,1
1223   test_multithrea     0.22   S         0         0  0,1
1224   test_multithrea     0.31   S         0         0  0,1
--------------------------------------------------------------------------------
Total threads: 5
```

## Quick Start

### Prerequisites

- **Docker** + **VS Code** with Remote - Containers extension
- Dev container includes everything: Ubuntu 24.04, kernel 6.8+ headers, build tools, static analysis

### Build and Run

1. Open project in VS Code → "Reopen in Container"
2. Build:
   ```bash
   make all
   ```
3. Install module:
   ```bash
   sudo make install
   ```
4. Run user program:
   ```bash
   ./build/proc_elf_ctrl
   ```

### Uninstall the Module

```bash
sudo make uninstall
```

## Project Structure

```
kernel_module/
├── .devcontainer/      # Dev container config (Docker + VS Code setup)
├── .github/            # CI/CD workflows (GitHub Actions)
├── docs/               # Detailed documentation
├── e2e/                # End-to-end testing scripts (QEMU setup, automation)
├── src/                # Source code (kernel module, user program, tests, helpers)
├── build/              # Build artifacts (generated by make)
└── Makefile            # Build system with quality checks
```

**Notes**: 
- **Memory Visualization**: Each region's bar length is proportional to its actual size
- Sizes are automatically displayed in appropriate units (B, KB, or MB)
- Low/High addresses show the memory address range of the process
- BSS_START and BSS_END may be equal (zero-length BSS) in modern ELF binaries. This is normal.
- **Open Sockets**: Shows file descriptor, socket family (AF_INET, AF_INET6, AF_UNIX), type (STREAM/DGRAM), state, and addresses
- Socket families: AF_INET (IPv4), AF_INET6 (IPv6), AF_UNIX (Unix domain), AF_NETLINK (Netlink)
- Thread STATE: R=Running, S=Sleeping, D=Uninterruptible, T=Stopped, t=Traced, Z=Zombie, X=Dead
- PRIORITY: Shown as nice value (-20 to 19, where lower is higher priority)
- CPU_AFFINITY: Shows which CPUs the thread can run on



## Safe Testing with QEMU

For maximum safety, test the kernel module in an isolated QEMU virtual machine that won't affect your host system.

### Quick Start

```bash
# One-time setup
./e2e/qemu-setup.sh

# Start VM
./e2e/qemu-run.sh

# In another terminal, run automated tests
./e2e/qemu-test.sh
```


## Makefile Targets

```bash
Build Targets:
  make all               - Build both kernel module and user program (default)
  make module            - Build kernel module only
  make user              - Build user program only
  make build-multithread - Build multi-threaded test program

Run Targets:
  make install           - Install kernel module (requires root)
  make uninstall         - Remove kernel module (requires root)
  make test              - Install module and run user program

Test Targets:
  make unit              - Build and run function-level unit tests
  make run-multithread   - Install module and test multi-thread program

Code Quality Targets:
  make check             - Run all static analysis checks
  make checkpatch        - Check kernel coding style with checkpatch.pl
  make sparse            - Run sparse static analyzer
  make cppcheck          - Run cppcheck static analyzer
  make format            - Format code with clang-format
  make format-check      - Check if code is properly formatted (CI)

Cleanup Targets:
  make clean             - Remove all build artifacts
```

## Testing

### Unit Tests (Recommended First)
```bash
make unit
```
Runs pure function tests without kernel dependencies.

### Multi-threaded Module Test
```bash
make run-multithread
```
Builds the multi-threaded test program, installs the module, and validates output via the user program.

### QEMU Testing (Safe Kernel Testing)
```bash
./e2e/qemu-setup.sh    # One-time setup
./e2e/qemu-run.sh      # Start VM
./e2e/qemu-test.sh     # Run automated tests
```

See [docs/TESTING.md](docs/TESTING.md) for detailed testing documentation.

## Documentation

- [TESTING.md](docs/TESTING.md) - Unit tests, QEMU testing, troubleshooting
- [TECHNICAL.md](docs/TECHNICAL.md) - Kernel module details, memory layout, limitations
- [CODE_QUALITY.md](docs/CODE_QUALITY.md) - Static analysis, code formatting, best practices
- [SCRIPTS.md](docs/SCRIPTS.md) - Detailed script documentation
- [RELEASE.md](docs/RELEASE.md) - Version release process and guidelines

## License

Dual BSD/GPL - Choose the license that works best for you:
- **GPL**: Required for Linux kernel compatibility
- **BSD**: Permissive for other uses

## Contributing

Contributions welcome! The project includes:
- Pre-configured dev container
- Automated testing (unit tests + QEMU E2E)
- Static analysis and formatting tools
- GitHub Actions CI/CD

---

**Educational Project**: Demonstrates Linux kernel module development. Use at your own risk.

### Configuration Files

- `.clang-format` - clang-format configuration (Linux kernel style)
- `.cppcheck-suppressions` - Suppression list for false positives
- `.editorconfig` - Editor configuration for consistent coding style

### Static Analysis Tools

#### checkpatch.pl
Official Linux kernel coding style checker. Enforces kernel coding standards including:
- Indentation and spacing rules
- Line length limits
- Function declaration style
- Comment formatting
- Macro usage patterns

#### sparse
Semantic parser specifically designed for kernel code. Detects:
- Type confusion errors
- Endianness issues
- Lock context imbalances
- Address space mismatches
- Null pointer dereferences

#### cppcheck
General-purpose C/C++ static analyzer. Finds:
- Memory leaks
- Buffer overflows
- Uninitialized variables
- Dead code
- Logic errors

#### clang-format
Code formatter that ensures consistent style:
- 8-space tabs (kernel standard)
- 80-column limit
- Linux brace style
- Proper spacing and alignment

## Testing

The module has been tested on:
- Ubuntu 20.04 LTS (Kernel 5.15+)
- Ubuntu 22.04 LTS (Kernel 5.19+)
- Ubuntu 24.04 LTS (Kernel 6.8+)

**Kernel Compatibility Notes:**
- Kernel 5.6+ required (proc_ops API)
- Kernel 6.8+ recommended (VMA iterator API)
- The code has been updated to use modern kernel APIs including VMA iterators and proc_ops**Educational Project**: Demonstrates Linux kernel module development. Use at your own risk.
