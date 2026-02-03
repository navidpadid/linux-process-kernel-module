// SPDX-License-Identifier: (GPL-2.0 OR BSD-2-Clause)
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "user_helpers.h"

int main(int argc, char **argv)
{
	FILE *fp;
	char pid_user[20];
	char buff[2048];

	if (argc > 1) {
		char *pid_path;
		char *det_path;
		size_t len;

		/* Safe string copy with explicit bounds checking */
		len = strlen(argv[1]);
		if (len >= sizeof(pid_user))
			len = sizeof(pid_user) - 1;
		memcpy(pid_user, argv[1], len);
		pid_user[len] = '\0';

		pid_path = build_proc_path("pid");
		fp = fopen(pid_path, "w");
		if (!fp) {
			perror("open pid");
			free(pid_path);
			return 1;
		}
		fprintf(fp, "%s", pid_user);
		fclose(fp);
		free(pid_path);

		det_path = build_proc_path("det");
		fp = fopen(det_path, "r");
		if (!fp) {
			perror("open det");
			free(det_path);
			return 1;
		}
		if (fgets(buff, sizeof(buff), fp))
			printf("%s\n", buff);
		if (fgets(buff, sizeof(buff), fp))
			printf("%s\n", buff);
		fclose(fp);
		free(det_path);
		return 0;
	}

	while (1) {
		char *pid_path2;
		char *det_path2;

		printf("************enter the process id:");
		if (scanf("%19s", pid_user) != 1) {
			fprintf(stderr, "invalid input\n");
			break;
		}

		pid_path2 = build_proc_path("pid");
		fp = fopen(pid_path2, "w");
		if (!fp) {
			perror("open pid");
			free(pid_path2);
			return 1;
		}
		fprintf(fp, "%s", pid_user);
		fclose(fp);
		free(pid_path2);

		printf("the process info is here:\n");
		det_path2 = build_proc_path("det");
		fp = fopen(det_path2, "r");
		if (!fp) {
			perror("open det");
			free(det_path2);
			return 1;
		}
		if (fgets(buff, sizeof(buff), fp))
			printf("%s\n", buff);
		if (fgets(buff, sizeof(buff), fp))
			printf("%s\n", buff);
		fclose(fp);
		free(det_path2);
	}
	return 0;
}
