/*
 *------program ptree_pchr.c--------
 *-------Done by Xia Hongchi--------
 *-----For Project 1, Problem 3-----
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    printf("520021910965Parent: %d\n", getpid());
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "fork error!\n");
        exit(1);
    }
    else if (pid == 0)
    {
        printf("520021910965Child: %d\n", getpid());
        execl("/data/misc/ptree", "ptree", (char *)0);
        fprintf(stderr, "execl error!\n");
        exit(1);
    }
    else
    {
        wait(NULL);
        exit(0);
    }
}