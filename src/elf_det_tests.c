// SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)
#include "elf_helpers.h"
#include <assert.h>
#include <stdio.h>

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

	puts("elf_helpers tests passed");
	return 0;
}
