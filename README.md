# Linux Process Information Kernel Module

[![CI](https://img.shields.io/github/actions/workflow/status/navidpadid/process-info-kernel-module/ci.yml?branch=main&style=for-the-badge&logo=github&logoColor=white&label=Build)](https://github.com/navidpadid/process-info-kernel-module/actions/workflows/ci.yml)
[![Last Commit](https://img.shields.io/github/last-commit/navidpadid/process-info-kernel-module?style=for-the-badge&logo=git&logoColor=white)](https://github.com/navidpadid/process-info-kernel-module/commits/main)
[![License](https://img.shields.io/badge/License-Dual%20BSD%2FGPL-blue?style=for-the-badge)](LICENSE)

> A Linux kernel module (LKM) with a user-space controller that extracts detailed process information including memory layout and ELF sections.

## Overview

This project implements a Linux Kernel Module that provides access to process information through the `/proc` filesystem. The module exposes process details such as:

- **Process ID (PID)** and **Process Name**
- **CPU Usage** statistics
- **Memory Layout**: Code, Data, and BSS sections
- **ELF Binary** information
- **Start/End addresses** for code and data segments

The project consists of two main components:
1. **Kernel Module** (`elf_det.c`) - Runs in kernel space and gathers process information
2. **User Program** (`proc_elf_ctrl.c`) - User-space controller to interact with the module

## Features

- **Process Memory Inspection**: View detailed memory layout of any running process
- **CPU Usage Tracking**: Real-time CPU usage percentage calculation
- **ELF Section Analysis**: Extract ELF binary sections (code, data, BSS)
- **Proc Filesystem Interface**: Easy interaction through `/proc/elf_det/`
- **Sequential File Operations**: Efficient data reading using kernel seq_file API
- **User-Friendly CLI**: Simple command-line interface for querying process information

## Project Structure

```
kernel_module/
├── .devcontainer/          # Development container configuration
│   ├── Dockerfile          # Container setup for kernel development
│   └── devcontainer.json   # VS Code dev container settings
├── .github/
│   └── workflows/
│       └── ci.yml          # GitHub Actions CI/CD pipeline
├── scripts/                # Testing and utility scripts
│   ├── qemu-setup.sh       # Setup QEMU testing environment
│   ├── qemu-run.sh         # Run QEMU VM
│   ├── qemu-test.sh        # Automated module testing in QEMU
│   └── README.md           # QEMU testing documentation
├── src/
│   ├── elf_det.c           # Kernel module source code
│   ├── proc_elf_ctrl.c     # User-space controller program
│   └── Kbuild              # Kernel build configuration
├── Makefile                # Build system
├── .gitignore              # Git ignore rules
└── README.md               # This file

Generated after build:
└── build/                  # Build artifacts (created by make)
    ├── elf_det.ko          # Compiled kernel module
    └── proc_elf_ctrl       # Compiled user program
```

## Prerequisites

### For Local Development

- **Linux Operating System** (Ubuntu 20.04+ recommended)
- **Kernel Headers** for your running kernel (Kernel 5.6+ required, 6.8+ recommended)
- **Build Tools**: gcc, make
- **Root Privileges** for module installation

### For Dev Container Development

- **Docker** installed and running
- **VS Code** with Remote - Containers extension
- **Internet connection** for initial setup

## Building and Running

### Option 1: Using Dev Container (Recommended)

1. Open the project in VS Code
2. Click "Reopen in Container" when prompted (or use Command Palette: "Remote-Containers: Reopen in Container")
3. Wait for the container to build and initialize
4. Build the project:

```bash
make all
```

### Option 2: Local Development

1. **Install prerequisites**:

```bash
sudo apt-get update
sudo apt-get install -y build-essential linux-headers-$(uname -r) kmod
```

2. **Build the kernel module and user program**:

```bash
make all
```

This will build:
- `build/elf_det.ko` - The kernel module
- `build/proc_elf_ctrl` - The user program

## Usage

### 1. Install the Kernel Module

```bash
sudo make install
```

This loads the module into the kernel. Verify it's loaded:

```bash
lsmod | grep elf_det
```

### 2. Run the User Program

```bash
./build/proc_elf_ctrl
```

The program will prompt you to enter a process ID (PID). You can find PIDs using:

```bash
ps aux | grep <process_name>
```

### 3. Example Output

```
***************************************************************************
******Navid user program for gathering memory info on desired process******
***************************************************************************
***************************************************************************
************enter the process id: 1234

the process info is here:
PID     NAME    CPU     START_CODE      END_CODE        START_DATA      END_DATA        BSS_START       BSS_END         ELF
01234   bash    0.5     0x0000563a1234  0x0000563a5678  0x0000563a9abc  0x0000563adef0  0x00007ffc1234  0x00007ffc5678  0x0000000000000040
```

### 4. Uninstall the Module

```bash
sudo make uninstall
```

## Safe Testing with QEMU

For maximum safety, test the kernel module in an isolated QEMU virtual machine that won't affect your host system.

### Quick Start

```bash
# One-time setup
./scripts/qemu-setup.sh

# Start VM
./scripts/qemu-run.sh

# In another terminal, run automated tests
./scripts/qemu-test.sh
```

### Requirements

```bash
# Ubuntu/Debian
sudo apt-get install qemu-system-x86 qemu-utils cloud-image-utils

# macOS
brew install qemu

# Fedora
sudo dnf install qemu-system-x86 qemu-img cloud-utils
```

### What QEMU Testing Does

- Downloads Ubuntu 24.04 VM image
- Configures VM with kernel headers
- Provides SSH access on port 2222
- Completely isolates module testing from your host
- Automated build, install, test, and uninstall cycle

See [scripts/README.md](scripts/README.md) for detailed QEMU testing documentation.

## Makefile Targets

| Target | Description |
|--------|-------------|
| `make all` | Build both kernel module and user program (default) |
| `make module` | Build kernel module only |
| `make user` | Build user program only |
| `make install` | Install kernel module (requires root) |
| `make uninstall` | Remove kernel module (requires root) |
| `make test` | Install module and run user program |
| `make unit` | Build and run function-level unit tests (no kernel required) |
| `make clean` | Remove all build artifacts |
| `make help` | Display help message |

## Technical Details

### Kernel Module (`elf_det.c`)

The kernel module creates entries in `/proc/elf_det/`:
- `/proc/elf_det/pid` - Write-only file to specify target PID
- `/proc/elf_det/det` - Read-only file to retrieve process information

**Key Functions:**
- `elfdet_show()` - Main function to gather and format process information
- `procfile_write()` - Handles PID input from user space
- `procfile_read()` - Returns formatted process data

**Kernel APIs Used:**
- `proc_fs.h` - Proc filesystem operations
- `seq_file.h` - Sequential file interface
- `sched.h` - Task/process structures
- `mm_types.h` - Memory management structures

### User Program (`proc_elf_ctrl.c`)

Simple C program that supports two modes:
1. Interactive: prompts for a PID, writes to `/proc/elf_det/pid`, then reads `/proc/elf_det/det` and prints two lines
2. Argument mode: run `./build/proc_elf_ctrl <PID>` to write the PID and print exactly two lines non-interactively

You can override the proc directory for testing with the environment variable `ELF_DET_PROC_DIR`.

Example:

```bash
ELF_DET_PROC_DIR=/tmp/fakeproc ./build/proc_elf_ctrl 12345
```

Internally, path construction is handled via helper `build_proc_path()`.

Helper headers used:
- `src/lib/user_helpers.h` – path building with env override
- `src/lib/elf_helpers.h` – pure functions for CPU usage and BSS range

## Testing

The module has been tested on:
- Ubuntu 20.04 LTS (Kernel 5.15+)
- Ubuntu 22.04 LTS (Kernel 5.19+)
- Ubuntu 24.04 LTS (Kernel 6.8+)

**Kernel Compatibility Notes:**
- Kernel 5.6+ required (proc_ops API)
- Kernel 6.8+ recommended (VMA iterator API)
- The code has been updated to use modern kernel APIs including VMA iterators and proc_ops

**Safe Testing Options:**
### Function-Level Unit Tests

Run pure function tests (no kernel required):

```bash
make unit
```

This builds and runs:
- `tests/elf_helpers_test.c` – verifies `compute_usage_permyriad()` and `compute_bss_range()`
- `tests/user_helpers_test.c` – verifies `build_proc_path()` with and without `ELF_DET_PROC_DIR`

Artifacts are created under `build/`.
1. **Dev Container** (current setup) - Isolated from host kernel
2. **QEMU VM** (recommended for extra safety) - See [scripts/README.md](scripts/README.md)
3. **Cloud VM** - Disposable testing environment

## Related Documentation

For detailed implementation guides, refer to the blog posts:

- **Part 2.1**: [Implementation of Simple "hello world" Module and Run](http://navidmalek.blog.ir/1396/07/07/Linux-2-1-Implementation-of-Simple-%E2%80%9Chello-world%E2%80%9D-Module-and-Run)
- **Part 2.2**: [Finding specified information and necessary functions](http://navidmalek.blog.ir/1396/07/07/Linux-2-2-Part-2-2-Finding-specified-information-and-necessary-functions-to-implement-desired-module)
- **Part 2.3**: [Implementation of kernel module and user program](http://navidmalek.blog.ir/1396/07/07/Linux-2-3-Part-2-3-Implementation-of-desired-kernel-module-and-user-program)

## Important Notes

- **Root privileges** are required to load/unload kernel modules
- Only works on **Linux** systems with kernel headers installed (Kernel 5.6+)
- **Kernel 6.8+** uses VMA iterators - older kernels may need code modifications
- Accessing invalid PIDs may cause undefined behavior
- Always unload the module before rebuilding

## Troubleshooting

### Module won't load

```bash
# Check kernel logs
dmesg | tail -n 20

# Verify kernel headers are installed
ls /lib/modules/$(uname -r)/build
```

### Build errors

```bash
# Install missing dependencies
sudo apt-get install -y build-essential linux-headers-$(uname -r)

# Clean and rebuild
make clean
make all
```

### Permission denied when running user program

```bash
# Ensure module is loaded
lsmod | grep elf_det

# Check proc entries exist
ls -la /proc/elf_det/
```

## License

This project is licensed under **Dual BSD/GPL** license.

## Author

**Navid Malek**

- Blog: [navidmalek.blog.ir](http://navidmalek.blog.ir)
- Project: [process-info-kernel-module](https://github.com/navidpadid/process-info-kernel-module)

## Contributing

Contributions, issues, and feature requests are welcome! Feel free to check the issues page.

## Changelog

### Version 1.0
- Initial release with basic process information extraction
- Support for CPU usage, memory layout, and ELF sections
- User-space controller program
- Dev container support
- CI/CD pipeline with GitHub Actions

---

**Note**: This is an educational project demonstrating Linux kernel module development. Use responsibly and at your own risk.
