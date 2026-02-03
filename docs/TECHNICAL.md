# Technical Details

## Kernel Module (`elf_det.c`)

The kernel module creates entries in `/proc/elf_det/`:
- `/proc/elf_det/pid` - Write-only file to specify target PID
- `/proc/elf_det/det` - Read-only file to retrieve process information
- `/proc/elf_det/threads` - Read-only file to retrieve thread information

### Key Functions

- `elfdet_show()` - Main function to gather and format process information
- `elfdet_threads_show()` - Gathers thread information for all threads in a process
- `find_stack_vma_end()` - Finds stack VMA lower boundary by iterating VMAs
- `procfile_write()` - Handles PID input from user space
- `procfile_read()` - Returns formatted process data

### Memory Information Extracted

| Field | Description | Source |
|-------|-------------|--------|
| **Code Section** | `start_code` to `end_code` | Executable code region (includes rodata) |
| **Data Section** | `start_data` to `end_data` | Initialized data region |
| **BSS Section** | `end_data` to `start_brk` | Uninitialized data region |
| **Heap Section** | `start_brk` to `brk` | brk-based dynamic memory allocation |
| **Stack** | `start_stack` to `stack_end` | Stack region (grows downward) |
| **ELF Base** | First VMA start | Base address of ELF binary (for PIE) |

### Important Notes and Limitations

#### 1. BSS May Be Zero-Length
Modern ELF binaries often have `end_data == start_brk`, resulting in zero-length BSS. This is **normal**, not an error.

#### 2. Read-Only Data (rodata)
The read-only data segment is typically merged with the code section (`start_code` to `end_code`) in modern binaries. It's not shown separately.

#### 3. Heap Limitation
The heap shown is **brk-based only** (traditional heap managed by brk/sbrk syscalls). Modern allocators like glibc's malloc also use:
- **mmap-based allocations** for large requests (>128KB typically)
- **Arena heaps** (multiple heap regions)
- These are NOT included in the brk-based heap range shown
- To see full heap usage, parse `/proc/pid/maps` for anonymous `[heap]` entries and unnamed mmap regions

#### 4. Stack
Shows both `start_stack` (top/base) and `stack_end` (current lower boundary). The stack grows downward from start_stack. The actual current stack pointer (in CPU registers) may be anywhere between these bounds.

### Kernel APIs Used

- `proc_fs.h` - Proc filesystem operations
- `seq_file.h` - Sequential file interface
- `sched.h` - Task/process structures
- `mm_types.h` - Memory management structures
- Maple tree API - Modern VMA iteration (kernel 6.8+)

## User Program (`proc_elf_ctrl.c`)

Simple C program that supports two modes:

### Interactive Mode
```bash
./build/proc_elf_ctrl
```
Prompts for a PID, writes to `/proc/elf_det/pid`, then reads `/proc/elf_det/det` and `/proc/elf_det/threads` and prints output.

### Argument Mode
```bash
./build/proc_elf_ctrl <PID>
```
Non-interactive mode - write PID and print both process and thread information.

### Environment Override

You can override the proc directory for testing:

```bash
ELF_DET_PROC_DIR=/tmp/fakeproc ./build/proc_elf_ctrl 12345
```

Internally, path construction is handled via `build_proc_path()`.

## Helper Libraries

### `src/elf_det.h`
Pure functions for CPU usage, BSS range, heap range, thread state, and address range checking:
- `compute_usage_permyriad()` - CPU usage calculation
- `compute_bss_range()` - BSS boundary validation
- `compute_heap_range()` - Heap boundary validation
- `is_address_in_range()` - Address containment check

Works in both kernel and user space contexts.

### `src/proc_elf_ctrl.h`
Path building with environment override:
- `build_proc_path()` - Constructs `/proc/elf_det/` paths with `ELF_DET_PROC_DIR` support

## Output Format

### Process Information (`/proc/elf_det/det`)
```
PID     NAME    CPU(%)  START_CODE      END_CODE        START_DATA      END_DATA        
BSS_START       BSS_END         HEAP_START      HEAP_END        STACK_START     
STACK_END       ELF_BASE
```

Example:
```
01234   bash    0.50    0x0000563a1234  0x0000563a5678  0x0000563a9abc  
0x0000563adef0  0x0000563adef0  0x0000563adef0  0x0000563b0000  
0x0000563b8000  0x00007ffd12345000  0x00007ffd12340000  0x0000563a1000
```

### Thread Information (`/proc/elf_det/threads`)
```
TID     NAME    CPU(%)  STATE   PRIORITY        NICE    CPU_AFF
```

Example:
```
01234   bash    0.50    S       0       0       0,1,2,3
01235   worker  0.01    R       0       0       0,1

Total threads: 2
```

#### Thread State Codes
- **R** - Running or runnable (on run queue)
- **S** - Interruptible sleep (waiting for an event)
- **D** - Uninterruptible sleep (usually I/O)
- **T** - Stopped (by job control signal)
- **t** - Tracing stop (by debugger)
- **Z** - Zombie (terminated but not reaped)
- **X** - Dead (should never be seen)

#### Thread Fields
- **TID** - Thread ID (same as PID for main thread)
- **NAME** - Thread name (typically same as process name)
- **CPU(%)** - Per-thread CPU usage since thread start
- **STATE** - Current thread state (see codes above)
- **PRIORITY** - Shown as nice value (-20 to 19, lower = higher priority)
- **NICE** - Nice value for the thread
- **CPU_AFF** - CPU affinity mask (which CPUs thread can run on)

**Note**: BSS_START and BSS_END may be equal (zero-length BSS) in modern ELF binaries. This is normal.
