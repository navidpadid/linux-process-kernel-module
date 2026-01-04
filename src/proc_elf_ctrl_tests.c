// SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)
#include "user_helpers.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	char *p1;
	char *p2;

	/* Without env override */
	unsetenv("ELF_DET_PROC_DIR");
	p1 = build_proc_path("pid");
	assert(p1 && strcmp(p1, "/proc/elf_det/pid") == 0);
	free(p1);

	/* With env override */
	setenv("ELF_DET_PROC_DIR", "/tmp/fakeproc", 1);
	p2 = build_proc_path("det");
	assert(p2 && strcmp(p2, "/tmp/fakeproc/det") == 0);
	free(p2);

	puts("user_helpers tests passed");
	return 0;
}
