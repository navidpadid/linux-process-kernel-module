/* SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause) */
#pragma once

#ifdef __KERNEL__
#include <linux/types.h>
typedef u64 eh_u64;
#else
#include <stdint.h>
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
