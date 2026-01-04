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

/* Compute BSS range from end_data and start_brk; returns 0 on invalid */
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
