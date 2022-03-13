/*
 *--------program ptree.c-----------
 *-------Done by Xia Hongchi--------
 *-----For Project 1, Problem 2-----
 */

#include <stdio.h>
#include <string.h>

struct prinfo
{
    pid_t parent_pid;       /* process id of parent */
    pid_t pid;              /* process id */
    pid_t first_child_pid;  /* pid of youngest child */
    pid_t next_sibling_pid; /* pid of older sibling */
    long state;             /* current state of process */
    long uid;               /* user id of process owner */
    char comm[16];          /* name of program executed */
    int layer;              /* layer of process */
};

int main()
{
    struct prinfo buf[100];
    int nr = 100, i;
    if (syscall(356, buf, &nr) == 1)
    {
        printf("Call failed\n");
    }
    else
    {
        for (i = 0; i < nr; i++)
        {
            int k;
            if (strcmp("Finish here!", buf[i].comm) == 0)
            {
                // printf("Finish pstree syscall!\n");
                break;
            }
            for (k = 0; k < buf[i].layer; k++)
                printf("    ");
            printf("%s,%d,%ld,%d,%d,%d,%ld\n", buf[i].comm, buf[i].pid, buf[i].state,
                   buf[i].parent_pid, buf[i].first_child_pid, buf[i].next_sibling_pid, buf[i].uid);
        }
    }
    return 0;
}