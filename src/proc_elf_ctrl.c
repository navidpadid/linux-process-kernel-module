#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *fp;
    char pid_user[20];
    char buff[2048];

    if (argc > 1) {
        strncpy(pid_user, argv[1], sizeof(pid_user) - 1);
        pid_user[sizeof(pid_user) - 1] = '\0';

        fp = fopen("/proc/elf_det/pid","w");
        if (!fp) {
            perror("open pid");
            return 1;
        }
        fprintf(fp,"%s", pid_user);
        fclose(fp);

        fp = fopen("/proc/elf_det/det","r");
        if (!fp) {
            perror("open det");
            return 1;
        }
        if (fgets(buff, sizeof(buff), fp))
            printf("%s\n", buff);
        if (fgets(buff, sizeof(buff), fp))
            printf("%s\n", buff);
        fclose(fp);
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

        fp = fopen("/proc/elf_det/pid","w");
        if (!fp) {
            perror("open pid");
            return 1;
        }
        fprintf(fp,"%s", pid_user);
        fclose(fp);

        printf("the process info is here:\n");
        fp = fopen("/proc/elf_det/det","r");
        if (!fp) {
            perror("open det");
            return 1;
        }
        if (fgets(buff, sizeof(buff), fp))
            printf("%s\n", buff);
        if (fgets(buff, sizeof(buff), fp))
            printf("%s\n", buff);
        fclose(fp);
    }
    return 0;
}