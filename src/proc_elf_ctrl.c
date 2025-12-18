#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
    FILE *fp;
    char pid_user[20];
    char buff[2048];

    printf("***************************************************************************\n");
    printf("******Navid user program for gathering memory info on desired process******\n");
    printf("***************************************************************************\n");
    printf("***************************************************************************\n");
    while(1==1){
    printf("************enter the process id:");
    scanf("%s",pid_user);
    
    fp = fopen("/proc/elf_det/pid","w");
    fprintf(fp,"%s", pid_user);
    fclose(fp);
    
    printf("the process info is here:\n");
    fp = fopen("/proc/elf_det/det","r");
    fgets(buff, 2048, (FILE*)fp);
    printf("%s\n",buff);
    fgets(buff, 2048, (FILE*)fp);
    printf("%s\n",buff);
    fclose(fp);
}
return 0;
}