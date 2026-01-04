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
        strncpy(pid_user, argv[1], sizeof(pid_user) - 1);
        pid_user[sizeof(pid_user) - 1] = '\0';

        char *pid_path = build_proc_path("pid");
        fp = fopen(pid_path, "w");
        if (!fp) {
            perror("open pid");
            free(pid_path);
            return 1;
        }
        fprintf(fp,"%s", pid_user);
        fclose(fp);
        free(pid_path);

        char *det_path = build_proc_path("det");
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

    printf("***************************************************************************\n");
    printf("******Navid user program for gathering memory info on desired process******\n");
    printf("***************************************************************************\n");
    printf("***************************************************************************\n");
    while (1) {
        printf("************enter the process id:");
        if (scanf("%19s", pid_user) != 1) {
            fprintf(stderr, "invalid input\n");
            break;
        }

        char *pid_path2 = build_proc_path("pid");
        fp = fopen(pid_path2, "w");
        if (!fp) {
            perror("open pid");
            free(pid_path2);
            return 1;
        }
        fprintf(fp,"%s", pid_user);
        fclose(fp);
        free(pid_path2);

        printf("the process info is here:\n");
        char *det_path2 = build_proc_path("det");
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