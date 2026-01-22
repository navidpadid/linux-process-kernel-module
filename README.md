# Linux Process Information Kernel Module

[![CI](https://img.shields.io/github/actions/workflow/status/navidpadid/process-info-kernel-module/ci.yml?branch=main&style=for-the-badge&logo=github&logoColor=white&label=Build)](https://github.com/navidpadid/process-info-kernel-module/actions/workflows/ci.yml)
[![Last Commit](https://img.shields.io/github/last-commit/navidpadid/process-info-kernel-module?style=for-the-badge&logo=git&logoColor=white)](https://github.com/navidpadid/process-info-kernel-module/commits/main)
[![License](https://img.shields.io/badge/License-Dual%20BSD%2FGPL-blue?style=for-the-badge)](LICENSE)

> A Linux kernel module (LKM) with a user-space controller that extracts detailed process information including memory layout and ELF sections.

## Overview

This project implements a Linux Kernel Module that provides access to process information through the `/proc` filesystem. The module exposes process details such as:

- **Process ID (PID)** and **Process Name**
- **CPU Usage** statistics
- **Memory Layout**: Code, Data, BSS, Heap, and Stack sections
- **ELF Binary** information
- **Start/End addresses** for code, data, BSS, heap, and stack segments

The project consists of two main components:
1. **Kernel Module** (`elf_det.c`) - Runs in kernel space and gathers process information
2. **User Program** (`proc_elf_ctrl.c`) - User-space controller to interact with the module

## Features

- **Process Memory Inspection**: View detailed memory layout of any running process
- **CPU Usage Tracking**: Real-time CPU usage percentage calculation
- **ELF Section Analysis**: Extract ELF binary sections (code, data, BSS)
- **Heap Analysis**: Track heap start and end addresses for dynamic memory allocation (brk-based)
- **Stack Tracking**: Monitor stack start address
- **Proc Filesystem Interface**: Easy interaction through `/proc/elf_det/`
- **Sequential File Operations**: Efficient data reading using kernel seq_file API
- **User-Friendly CLI**: Simple command-line interface for querying process information

## Project Structure

```
kernel_module/
├── .devcontainer/          # Development container configuration
│   ├── Dockerfile          # Pre-installs: clang-format, sparse, cppcheck
│   └── devcontainer.json   # Auto-configures Git hooks on startup
├── .github/
│   └── workflows/
│       └── ci.yml          # GitHub Actions CI/CD with static analysis
├── scripts/                # Testing and utility scripts
│   ├── qemu-setup.sh       # Setup QEMU testing environment
│   ├── qemu-run.sh         # Run QEMU VM
│   ├── qemu-test.sh        # Automated module testing in QEMU
│   ├── pre-commit.sh       # Pre-commit hook script (auto-installed in container)
│   └── README.md           # Scripts documentation
├── src/
│   ├── elf_det.c           # Kernel module source code
│   ├── proc_elf_ctrl.c     # User-space controller program
│   ├── elf_det_tests.c     # Unit tests for elf_det functions
│   ├── proc_elf_ctrl_tests.c  # Unit tests for proc_elf_ctrl helpers
│   ├── elf_helpers.h       # Helper functions for CPU usage and BSS range
│   ├── user_helpers.h      # Helper functions for path building
│   └── Kbuild              # Kernel build configuration
├── .clang-format           # Code formatting configuration
├── .cppcheck-suppressions  # Static analysis suppressions
├── .editorconfig           # Editor configuration
├── Makefile                # Build system with static analysis targets
├── .gitignore              # Git ignore rules
└── README.md               # This file

Generated after build:
└── build/                  # Build artifacts (created by make)
    ├── elf_det.ko          # Compiled kernel module
    ├── proc_elf_ctrl       # Compiled user program
    ├── elf_det_tests       # Compiled unit tests for elf_det
    └── proc_elf_ctrl_tests # Compiled unit tests for proc_elf_ctrl
```

## Prerequisites

This project uses a **Dev Container** for development to ensure a consistent, fully-configured environment.

**Required:**
- **Docker** installed and running
- **VS Code** with Remote - Containers extension
- **Internet connection** for initial container build

**Included in container:**
-  Ubuntu 24.04 with Kernel 6.8+ headers
-  All build tools (gcc, make, kernel headers)
-  Static analysis tools (clang-format, sparse, cppcheck) pre-installed
-  Git pre-commit hooks automatically configured
-  Zero manual configuration required

## Building and Running

### Using Dev Container

1. Open the project in VS Code
2. Click "Reopen in Container" when prompted (or use Command Palette: "Remote-Containers: Reopen in Container")
3. Wait for the container to build and initialize (first time only)
4. Build the project:

```bash
make all
```

This will build:
- `build/elf_det.ko` - The kernel module
- `build/proc_elf_ctrl` - The user program

All dependencies, tools, and hooks are pre-configured automatically!

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
PID     NAME    CPU(%)  START_CODE      END_CODE        START_DATA      END_DATA        BSS_START       BSS_END         HEAP_START      HEAP_END        STACK_START     STACK_END       ELF_BASE
01234   bash    0.50    0x0000563a1234  0x0000563a5678  0x0000563a9abc  0x0000563adef0  0x0000563adef0  0x0000563adef0  0x0000563b0000  0x0000563b8000  0x00007ffd12345000  0x00007ffd12340000  0x0000563a1000
```

**Note**: BSS_START and BSS_END may be equal (zero-length BSS) in modern ELF binaries. This is normal.

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


### What QEMU Testing Does

- Downloads Ubuntu 24.04 VM image
- Configures VM with kernel headers
- Provides SSH access on port 2222
- Completely isolates module testing from your host
- Automated build, install, test, and uninstall cycle

See [scripts/README.md](scripts/README.md) for detailed QEMU testing documentation.

## Makefile Targets

### Build Targets

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

### Code Quality Targets

| Target | Description |
|--------|-------------|
| `make check` | Run all static analysis checks (checkpatch + sparse + cppcheck) |
| `make checkpatch` | Check kernel coding style with checkpatch.pl |
| `make sparse` | Run sparse static analyzer for kernel code |
| `make cppcheck` | Run cppcheck static analyzer for C/C++ |
| `make format` | Format all source files with clang-format |
| `make format-check` | Check if code is properly formatted (CI-friendly) |

## Technical Details

### Kernel Module (`elf_det.c`)

The kernel module creates entries in `/proc/elf_det/`:
- `/proc/elf_det/pid` - Write-only file to specify target PID
- `/proc/elf_det/det` - Read-only file to retrieve process information

**Key Functions:**
- `elfdet_show()` - Main function to gather and format process information
- `procfile_write()` - Handles PID input from user space
- `procfile_read()` - Returns formatted process data

**Memory Information Extracted:**
- **Code Section**: `start_code` to `end_code` - executable code region (includes rodata)
- **Data Section**: `start_data` to `end_data` - initialized data region
- **BSS Section**: `end_data` to `start_brk` - uninitialized data region
- **Heap Section**: `start_brk` to `brk` - brk-based dynamic memory allocation
- **Stack**: `start_stack` (top/base) to `stack_end` (current lower bound)
- **ELF Base**: Base address of the ELF binary (first VMA, typically lower than start_code for PIE)

**Important Notes and Limitations:**

1. **BSS May Be Zero-Length**: Modern ELF binaries often have `end_data == start_brk`, resulting in zero-length BSS. This is normal, not an error.

2. **Read-Only Data (rodata)**: The read-only data segment is typically merged with the code section (`start_code` to `end_code`) in modern binaries. It's not shown separately.

3. **Heap Limitation**: The heap shown is **brk-based only** (traditional heap managed by brk/sbrk syscalls). Modern allocators like glibc's malloc also use:
   - **mmap-based allocations** for large requests (>128KB typically)
   - **Arena heaps** (multiple heap regions)
   - These are NOT included in the brk-based heap range shown
   - To see full heap usage, parse `/proc/pid/maps` for anonymous `[heap]` entries and unnamed mmap regions

4. **Stack**: Shows both `start_stack` (top/base) and `stack_end` (current lower boundary). The stack grows downward from start_stack. The actual current stack pointer (in CPU registers) may be anywhere between these bounds.

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
- `src/user_helpers.h` – path building with env override
- `src/elf_helpers.h` – pure functions for CPU usage, BSS range, and heap range

## Code Quality and Static Analysis

This project includes comprehensive static analysis and code formatting tools to ensure high code quality and compliance with Linux kernel coding standards.

### Installing Analysis Tools

Run the installation script to install all required tools:

```bash
./scripts/install-analysis-tools.sh
```

This installs:
- **clang-format**: Code formatting according to kernel style
- **sparse**: Semantic parser for C, designed for kernel code
- **cppcheck**: General C/C++ static analysis
- **checkpatch.pl**: Official kernel coding style checker (requires kernel sources)

### Running Static Analysis

Run all checks at once:

```bash
make check
```

Or run individual checks:

```bash
# Check kernel coding style
make checkpatch

# Run sparse static analyzer
make sparse

# Run cppcheck analyzer
make cppcheck
```

### Code Formatting

Format all source files automatically:

```bash
make format
```

Check if code is properly formatted (useful for CI/CD):

```bash
make format-check
```

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
- The code has been updated to use modern kernel APIs including VMA iterators and proc_ops

**Safe Testing Options:**
### Function-Level Unit Tests

Run pure function tests (no kernel required):

```bash
make unit
```

This builds and runs:
- `src/elf_det_tests.c` – verifies `compute_usage_permyriad()`, `compute_bss_range()`, and `compute_heap_range()`
- `src/proc_elf_ctrl_tests.c` – verifies `build_proc_path()` with and without `ELF_DET_PROC_DIR`

Artifacts are created under `build/`.
1. **Dev Container** (current setup) - Isolated from host 
2. **QEMU VM** (recommended for extra safety from kernel) - See [scripts/README.md](scripts/README.md)
3. **Cloud VM** - Disposable testing environment


## Important Notes

- **Use the dev container** for development - it provides a consistent, fully-configured environment
- **Root privileges** are required to load/unload kernel modules
- The container includes **Ubuntu 24.04 with Kernel 6.8+** (VMA iterator API support)
- Accessing invalid PIDs may cause undefined behavior
- Always unload the module before rebuilding
- **Static analysis tools** and **Git hooks** are automatically configured on container startup

## Static Analysis and Code Quality

This project includes comprehensive static analysis and code formatting tools to ensure high code quality and compliance with Linux kernel coding standards.

**All tools and hooks are automatically installed and configured in the dev container.** No manual setup required!

### Tools Integrated

The following tools are included:

#### 1. clang-format
**Purpose**: Automatic code formatting  
**Standard**: Linux kernel coding style  
**Configuration**: `.clang-format`

**Features**:
- 8-space tabs (kernel standard)
- 80-column line limit
- Linux brace style
- Consistent spacing and alignment

**Usage**:
```bash
make format          # Format all files
make format-check    # Check formatting (CI-friendly)
```

#### 2. checkpatch.pl
**Purpose**: Kernel coding style compliance  
**Source**: Official Linux kernel scripts  

**Checks**:
- Indentation and spacing rules
- Line length limits (80 columns preferred)
- Function declaration style
- Comment formatting (/* */ style)
- Macro usage patterns
- Variable naming conventions

**Usage**:
```bash
make checkpatch
```

#### 3. sparse
**Purpose**: Semantic analysis for C code  
**Specialty**: Kernel-specific checks  

**Detects**:
- Type confusion errors
- Endianness issues (`__be32`, `__le32`)
- Lock context imbalances
- Address space mismatches (`__user`, `__kernel`)
- Null pointer dereferences

**Usage**:
```bash
make sparse
```

#### 4. cppcheck
**Purpose**: General C/C++ static analysis  
**Configuration**: `.cppcheck-suppressions`

**Detects**:
- Memory leaks
- Buffer overflows
- Uninitialized variables
- Dead code
- Logic errors

**Usage**:
```bash
make cppcheck
```

### Running Static Analysis

Run all checks at once:
```bash
make check
```

Or run individual checks:
```bash
make checkpatch  # Kernel coding style
make sparse      # Kernel static analysis
make cppcheck    # General C/C++ analysis
make format      # Format all code
```

### Configuration Files

- **`.clang-format`** - Code formatting rules (Linux kernel style)
- **`.cppcheck-suppressions`** - Suppression list for false positives
- **`.editorconfig`** - Editor configuration for consistent coding style

### Git Hooks

Pre-commit hooks are automatically installed on container startup.

The pre-commit hook runs:
- Code formatting checks
- Cppcheck static analysis
- Checkpatch coding style validation

To bypass hooks (use sparingly):
```bash
git commit --no-verify -m "message"
```

### Best Practices

**Follow Linux Kernel Coding Style:**
- Use tabs (8 spaces), not spaces for indentation
- 80 column limit for code
- Opening brace on same line (except functions)
- Space after keywords: `if (`, `while (`, `for (`
- No space after function names: `function(arg)`
- Use C89-style comments: `/* comment */`

**Example:**
```c
int example_function(int param)
{
	if (param > 0) {
		/* Comment style: C89 */
		return param * 2;
	}
	return 0;
}
```

**Development Workflow:**

```bash
# Everything is ready - just start coding!

# Format code before committing
make format

# Run all checks
make check

# Commit (hooks run automatically)
git commit -m "Your message"
```

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

**Navid Malekghaini**

## Contributing

Contributions, issues, and feature requests are welcome!

## Changelog

### Version 1.3
- Added `STACK_END` extraction by parsing stack VMA
- Fixed `ELF` column to show actual ELF base address (renamed to `ELF_BASE`)
- Clarified that ELF_BASE is the first VMA (PIE binary base)
- Stack now shows full range (start to end)
- Improved VMA iteration for stack boundary detection

### Version 1.2.1
- Added stack start address extraction (`STACK_START` column)
- Improved documentation of memory layout limitations
- Added comprehensive notes about BSS zero-length in modern binaries
- Documented brk-based heap limitation (mmap heaps not tracked)
- Clarified that rodata is merged with code section
- Enhanced code comments for better understanding
- Fixed potential confusion about "always zero" BSS

### Version 1.2
- Added heap start and end address extraction
- Enhanced memory layout analysis with heap region tracking
- Added `compute_heap_range()` helper function
- Updated unit tests to cover heap range computation
- Improved documentation with comprehensive heap information
- Updated output format to include HEAP_START and HEAP_END columns

### Version 1.1
- Integrated static analysis tools (clang-format, sparse, cppcheck, checkpatch)
- Automated Git pre-commit hooks for code quality
- Dev container with zero-configuration setup
- Enhanced CI/CD pipeline with static analysis checks
- Comprehensive code quality documentation

### Version 1.0
- Initial release with basic process information extraction
- Support for CPU usage, memory layout, and ELF sections
- User-space controller program
- Function-level unit tests for core functionality
- Dev container support for isolated development
- CI/CD pipeline with GitHub Actions
- QEMU testing environment for safe kernel module testing
- Comprehensive documentation and quick reference guides
- Dual BSD/GPL license

---

**Note**: This is an educational project demonstrating Linux kernel module development. Use responsibly and at your own risk.
