/* SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause) */
#pragma once

#ifdef __KERNEL__
#include <linux/types.h>
typedef u64 eh_u64;
#else
#include <stdint.h>
#include <stdio.h>
typedef uint64_t eh_u64;
#endif

/* Compute CPU usage permyriad (percent * 100) from total_ns and delta_ns */
static inline eh_u64 compute_usage_permyriad(eh_u64 total_ns, eh_u64 delta_ns)
{
	if (delta_ns == 0)
		return 0;
	return (10000ULL * total_ns) / delta_ns;
}

/* Compute BSS range from end_data and start_brk; returns 0 on invalid
 * BSS (Block Started by Symbol): Uninitialized data segment
 * Note: Modern ELF binaries may have zero-length BSS if end_data == start_brk
 * This is normal and not an error.
 */
static inline int compute_bss_range(unsigned long end_data,
				    unsigned long start_brk,
				    unsigned long *out_start,
				    unsigned long *out_end)
{
	if (start_brk < end_data) {
		*out_start = 0;
		*out_end = 0;
		return 0;
	}
	*out_start = end_data;
	*out_end = start_brk;
	return 1;
}

/* Compute heap range from start_brk and brk; returns 0 on invalid
 * Heap: Dynamic memory allocation region
 * LIMITATION: This only tracks brk-based heap (traditional heap).
 * Modern allocators (glibc malloc) also use mmap for large allocations
 * and arena-based heaps, which are NOT included in this range.
 * To see full heap usage, you would need to parse /proc/pid/maps for
 * anonymous mappings marked as [heap] or unnamed mmap regions.
 */
static inline int compute_heap_range(unsigned long start_brk, unsigned long brk,
				     unsigned long *out_start,
				     unsigned long *out_end)
{
	if (brk < start_brk) {
		*out_start = 0;
		*out_end = 0;
		return 0;
	}
	*out_start = start_brk;
	*out_end = brk;
	return 1;
}

/* Check if an address falls within a memory range (inclusive)
 * Used for finding VMAs that contain specific addresses like stack
 * Returns 1 if addr is within [range_start, range_end), 0 otherwise
 */
static inline int is_address_in_range(unsigned long addr,
				      unsigned long range_start,
				      unsigned long range_end)
{
	if (range_start > range_end)
		return 0;
	if (addr >= range_start && addr < range_end)
		return 1;
	return 0;
}

/* Convert thread/task state value to single character representation
 * Kernel state constants (only available in kernel context):
 * - TASK_RUNNING (0x0000) -> 'R'
 * - TASK_INTERRUPTIBLE (0x0001) -> 'S'
 * - TASK_UNINTERRUPTIBLE (0x0002) -> 'D'
 * - __TASK_STOPPED (0x0004) -> 'T'
 * - __TASK_TRACED (0x0008) -> 't'
 * - EXIT_ZOMBIE (0x0020) -> 'Z'
 * - EXIT_DEAD (0x0040) -> 'X'
 * For testing: pass raw state values (0, 1, 2, 4, 8, 32, 64)
 */
static inline char get_thread_state_char(unsigned long state)
{
	switch (state) {
	case 0x0000: /* TASK_RUNNING */
		return 'R';
	case 0x0001: /* TASK_INTERRUPTIBLE */
		return 'S';
	case 0x0002: /* TASK_UNINTERRUPTIBLE */
		return 'D';
	case 0x0004: /* __TASK_STOPPED */
		return 'T';
	case 0x0008: /* __TASK_TRACED */
		return 't';
	case 0x0020: /* EXIT_ZOMBIE */
		return 'Z';
	case 0x0040: /* EXIT_DEAD */
		return 'X';
	default:
		return '?';
	}
}

/* Build CPU affinity mask string from bitmap
 * Checks up to max_cpus and appends CPU numbers (comma-separated)
 * Returns number of characters written (excluding null terminator)
 * If no CPUs are set, writes "none"
 *
 * Parameters:
 *   cpu_mask: array representing CPU bitmap (1 = CPU available, 0 = not)
 *   max_cpus: maximum number of CPUs to check
 *   out_buf: output buffer for the affinity string
 *   buf_size: size of output buffer
 */
static inline int build_cpu_affinity_string(const int *cpu_mask, int max_cpus,
					    char *out_buf, int buf_size)
{
	int i, len = 0;
	int has_cpu = 0;

	if (!out_buf || buf_size < 5)
		return 0;

	for (i = 0; i < max_cpus && len < buf_size - 2; i++) {
		if (cpu_mask[i]) {
			has_cpu = 1;
			len +=
			    snprintf(out_buf + len, buf_size - len, "%d,", i);
		}
	}

	if (has_cpu && len > 0) {
		out_buf[len - 1] = '\0'; /* Remove trailing comma */
		return len - 1;
	}

	snprintf(out_buf, buf_size, "none");
	return 4;
}
