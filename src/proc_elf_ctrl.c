// SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "user_helpers.h"

static void print_process_info(const char *pid_str)
{
	FILE *fp;
	char buff[2048];
	char *pid_path;
	char *det_path;
	char *threads_path;

	/* Write PID to proc file */
	pid_path = build_proc_path("pid");
	fp = fopen(pid_path, "w");
	if (!fp) {
		perror("open pid");
		free(pid_path);
		return;
	}
	fprintf(fp, "%s", pid_str);
	fclose(fp);
	free(pid_path);

	/* Read and display process info */
	printf("\n");
	printf("==============================================================="
	       "=================\n");
	printf("                          PROCESS INFORMATION                  "
	       "                 \n");
	printf("==============================================================="
	       "=================\n");
	det_path = build_proc_path("det");
	fp = fopen(det_path, "r");
	if (!fp) {
		perror("open det");
		free(det_path);
		return;
	}
	while (fgets(buff, sizeof(buff), fp))
		printf("%s", buff);
	fclose(fp);
	free(det_path);

	/* Read and display thread info */
	printf("\n");
	printf("==============================================================="
	       "=================\n");
	printf("                          THREAD INFORMATION                   "
	       "                 \n");
	printf("==============================================================="
	       "=================\n");
	threads_path = build_proc_path("threads");
	fp = fopen(threads_path, "r");
	if (!fp) {
		perror("open threads");
		free(threads_path);
		return;
	}
	while (fgets(buff, sizeof(buff), fp))
		printf("%s", buff);
	fclose(fp);
	free(threads_path);
	printf("==============================================================="
	       "=================\n");
}

int main(int argc, char **argv)
{
	char pid_user[20];

	if (argc > 1) {
		size_t len;

		/* Safe string copy with explicit bounds checking */
		len = strlen(argv[1]);
		if (len >= sizeof(pid_user))
			len = sizeof(pid_user) - 1;
		memcpy(pid_user, argv[1], len);
		pid_user[len] = '\0';

		print_process_info(pid_user);
		return 0;
	}

	while (1) {
		printf("\n>> Enter process ID (or Ctrl+C to exit): ");
		if (scanf("%19s", pid_user) != 1) {
			fprintf(stderr, "invalid input\n");
			break;
		}

		print_process_info(pid_user);
	}
	return 0;
}
