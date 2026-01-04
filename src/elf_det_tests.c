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
	int ret1, ret2;

	ret1 = compute_bss_range(1000UL, 2000UL, &s, &e);
	assert(ret1 == 1);
	assert(s == 1000UL && e == 2000UL);

	ret2 = compute_bss_range(3000UL, 2000UL, &s, &e);
	assert(ret2 == 0);
	assert(s == 0UL && e == 0UL);

	puts("elf_helpers tests passed");
	return 0;
}
