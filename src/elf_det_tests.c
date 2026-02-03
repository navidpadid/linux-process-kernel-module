// SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)
#include "elf_helpers.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
	/* compute_usage_permyriad tests */
	assert(compute_usage_permyriad(0, 1000000ULL) == 0);
	assert(compute_usage_permyriad(500000ULL, 1000000ULL) == 5000ULL);
	assert(compute_usage_permyriad(250000ULL, 1000000ULL) == 2500ULL);
	assert(compute_usage_permyriad(1000000ULL, 1000000ULL) == 10000ULL);
	assert(compute_usage_permyriad(0, 0) == 0);

	/* compute_bss_range tests */
	unsigned long s = 0, e = 0;
	int ret1, ret2, ret3, ret4;

	ret1 = compute_bss_range(1000UL, 2000UL, &s, &e);
	assert(ret1 == 1);
	assert(s == 1000UL && e == 2000UL);

	ret2 = compute_bss_range(3000UL, 2000UL, &s, &e);
	assert(ret2 == 0);
	assert(s == 0UL && e == 0UL);

	/* compute_heap_range tests */
	ret3 = compute_heap_range(5000UL, 8000UL, &s, &e);
	assert(ret3 == 1);
	assert(s == 5000UL && e == 8000UL);

	ret4 = compute_heap_range(9000UL, 7000UL, &s, &e);
	assert(ret4 == 0);
	assert(s == 0UL && e == 0UL);

	/* Test heap with same start and end (empty heap) */
	ret1 = compute_heap_range(10000UL, 10000UL, &s, &e);
	assert(ret1 == 1);
	assert(s == 10000UL && e == 10000UL);

	/* is_address_in_range tests */
	/* Test address within range */
	assert(is_address_in_range(5000UL, 1000UL, 10000UL) == 1);
	assert(is_address_in_range(1000UL, 1000UL, 10000UL) == 1);

	/* Test address at boundary (range_end is exclusive) */
	assert(is_address_in_range(10000UL, 1000UL, 10000UL) == 0);

	/* Test address outside range */
	assert(is_address_in_range(500UL, 1000UL, 10000UL) == 0);
	assert(is_address_in_range(15000UL, 1000UL, 10000UL) == 0);

	/* Test invalid range (start > end) */
	assert(is_address_in_range(5000UL, 10000UL, 1000UL) == 0);

	/* Test edge cases */
	assert(is_address_in_range(0UL, 0UL, 1UL) == 1);
	assert(is_address_in_range(ULONG_MAX - 1, 0UL, ULONG_MAX) == 1);

	/* get_thread_state_char tests */
	assert(get_thread_state_char(0x0000) == 'R'); /* TASK_RUNNING */
	assert(get_thread_state_char(0x0001) == 'S'); /* TASK_INTERRUPTIBLE */
	assert(get_thread_state_char(0x0002) == 'D'); /* TASK_UNINTERRUPTIBLE */
	assert(get_thread_state_char(0x0004) == 'T'); /* __TASK_STOPPED */
	assert(get_thread_state_char(0x0008) == 't'); /* __TASK_TRACED */
	assert(get_thread_state_char(0x0020) == 'Z'); /* EXIT_ZOMBIE */
	assert(get_thread_state_char(0x0040) == 'X'); /* EXIT_DEAD */
	assert(get_thread_state_char(0x9999) == '?'); /* Unknown state */
	assert(get_thread_state_char(0xFFFF) == '?'); /* Unknown state */

	/* build_cpu_affinity_string tests */
	char buf[64];
	int len;
	int mask1[8] = {1, 0, 1, 0, 1, 0, 1, 0}; /* CPUs 0,2,4,6 */
	int mask2[8] = {1, 1, 1, 1, 1, 1, 1, 1}; /* All CPUs */
	int mask3[8] = {0, 0, 0, 0, 0, 0, 0, 0}; /* No CPUs */
	int mask4[8] = {0, 0, 0, 0, 0, 0, 0, 1}; /* Only CPU 7 */
	int mask5[8] = {1, 0, 0, 0, 0, 0, 0, 0}; /* Only CPU 0 */

	/* Test normal mask */
	len = build_cpu_affinity_string(mask1, 8, buf, sizeof(buf));
	assert(len == 7); /* "0,2,4,6" */
	assert(strcmp(buf, "0,2,4,6") == 0);

	/* Test all CPUs */
	len = build_cpu_affinity_string(mask2, 8, buf, sizeof(buf));
	assert(len == 15); /* "0,1,2,3,4,5,6,7" */
	assert(strcmp(buf, "0,1,2,3,4,5,6,7") == 0);

	/* Test no CPUs */
	len = build_cpu_affinity_string(mask3, 8, buf, sizeof(buf));
	assert(len == 4); /* "none" */
	assert(strcmp(buf, "none") == 0);

	/* Test single CPU at end */
	len = build_cpu_affinity_string(mask4, 8, buf, sizeof(buf));
	assert(len == 1); /* "7" */
	assert(strcmp(buf, "7") == 0);

	/* Test single CPU at start */
	len = build_cpu_affinity_string(mask5, 8, buf, sizeof(buf));
	assert(len == 1); /* "0" */
	assert(strcmp(buf, "0") == 0);

	/* Test with smaller max_cpus */
	len = build_cpu_affinity_string(mask1, 4, buf, sizeof(buf));
	assert(len == 3); /* "0,2" */
	assert(strcmp(buf, "0,2") == 0);

	/* Test with small buffer */
	char small_buf[6];
	len = build_cpu_affinity_string(mask1, 8, small_buf, sizeof(small_buf));
	assert(len >= 0); /* Should handle gracefully */
	assert(strlen(small_buf) < sizeof(small_buf));

	/* Test NULL buffer */
	len = build_cpu_affinity_string(mask1, 8, NULL, 64);
	assert(len == 0);

	/* Test too small buffer */
	len = build_cpu_affinity_string(mask1, 8, buf, 3);
	assert(len == 0);

	puts("elf_helpers tests passed");
	return 0;
}
