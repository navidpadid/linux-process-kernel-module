// SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)
#include "elf_det.h"
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
	const int mask1[8] = {1, 0, 1, 0, 1, 0, 1, 0}; /* CPUs 0,2,4,6 */
	const int mask2[8] = {1, 1, 1, 1, 1, 1, 1, 1}; /* All CPUs */
	const int mask3[8] = {0, 0, 0, 0, 0, 0, 0, 0}; /* No CPUs */
	const int mask4[8] = {0, 0, 0, 0, 0, 0, 0, 1}; /* Only CPU 7 */
	const int mask5[8] = {1, 0, 0, 0, 0, 0, 0, 0}; /* Only CPU 0 */

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

	/* format_size_with_unit tests */
	char size_buf[32];

	/* Test bytes */
	len = format_size_with_unit(512, size_buf, sizeof(size_buf));
	assert(strcmp(size_buf, "512 B") == 0);
	assert(len > 0);

	/* Test kilobytes */
	len = format_size_with_unit(1024, size_buf, sizeof(size_buf));
	assert(strcmp(size_buf, "1 KB") == 0);
	assert(len > 0);

	len = format_size_with_unit(2048, size_buf, sizeof(size_buf));
	assert(strcmp(size_buf, "2 KB") == 0);

	/* Test megabytes */
	len = format_size_with_unit(1024 * 1024, size_buf, sizeof(size_buf));
	assert(strcmp(size_buf, "1 MB") == 0);
	assert(len > 0);

	len =
	    format_size_with_unit(5 * 1024 * 1024, size_buf, sizeof(size_buf));
	assert(strcmp(size_buf, "5 MB") == 0);

	/* Test edge cases */
	len = format_size_with_unit(0, size_buf, sizeof(size_buf));
	assert(strcmp(size_buf, "0 B") == 0);

	len = format_size_with_unit(1023, size_buf, sizeof(size_buf));
	assert(strcmp(size_buf, "1023 B") == 0);

	/* calculate_bar_width tests */
	int width;

	/* Test normal proportions */
	width = calculate_bar_width(100, 1000, 50);
	assert(width == 5); /* 100/1000 * 50 = 5 */

	width = calculate_bar_width(500, 1000, 50);
	assert(width == 25); /* Half */

	width = calculate_bar_width(1000, 1000, 50);
	assert(width == 50); /* Full */

	/* Test minimum width for non-zero sizes */
	width = calculate_bar_width(1, 1000000, 50);
	assert(width == 1); /* Should be at least 1 for non-zero */

	/* Test zero size */
	width = calculate_bar_width(0, 1000, 50);
	assert(width == 0);

	/* Test zero total (edge case) */
	width = calculate_bar_width(100, 0, 50);
	assert(width == 0);

	/* generate_region_visualization tests */
	struct memory_region region;
	char viz_buf[256];

	/* Test normal region */
	region.name = "CODE";
	region.size = 1024 * 1024; /* 1 MB */
	region.exists = 1;
	len = generate_region_visualization(&region, 25, 50, viz_buf,
					    sizeof(viz_buf));
	assert(len > 0);
	assert(strstr(viz_buf, "CODE") != NULL);
	assert(strstr(viz_buf, "1 MB") != NULL);
	assert(strstr(viz_buf, "[=========================") != NULL);

	/* Test small region */
	region.name = "DATA";
	region.size = 512; /* 512 B */
	region.exists = 1;
	len = generate_region_visualization(&region, 5, 50, viz_buf,
					    sizeof(viz_buf));
	assert(len > 0);
	assert(strstr(viz_buf, "DATA") != NULL);
	assert(strstr(viz_buf, "512 B") != NULL);

	/* Test non-existent region */
	region.name = "BSS";
	region.size = 0;
	region.exists = 0;
	len = generate_region_visualization(&region, 0, 50, viz_buf,
					    sizeof(viz_buf));
	assert(len == 0); /* Should return 0 for non-existent regions */

	/* Test zero size but exists flag set */
	region.name = "HEAP";
	region.size = 0;
	region.exists = 1;
	len = generate_region_visualization(&region, 0, 50, viz_buf,
					    sizeof(viz_buf));
	assert(len == 0); /* Should return 0 for zero size */

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
