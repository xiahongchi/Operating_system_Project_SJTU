#include <sched.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main(){

    pid_t pid;
    struct timespec t;
    int choice,sched_algo,sched_prio,cur_sched_algo,pre_sched_algo,wcounts,ret;
    struct sched_param param;
    param.sched_priority=0;
    printf("The task you want to perform: \n1-change scheduler\n2-trace interval\n3-change wcounts (ONLY For TEST!)\nInput here: ");
    scanf("%d",&choice);
    switch (choice){
    case 1:
        printf("please input which process you want to change: pid = ");
        scanf("%d",&pid);
        printf("Select the scheduling algorithms: 1-FIFO, 2-RR, 6-RAS: ");
        scanf("%d",&sched_algo);

        switch (sched_algo){
            case 1:
            case 2:
                printf("Select priority (1-99) : ");
                scanf("%d",&sched_prio);
                if(sched_prio <= 0 || sched_prio > 99){
                    printf("invalid input!\n");
                    exit(-1);
                }
                param.sched_priority = sched_prio;
                break;
            case 6:
                break;
            
            default:
                printf("invalid input!\n");
                exit(-1);
                break;
        }

        pre_sched_algo = sched_getscheduler(pid);
        ret=sched_setscheduler(pid,sched_algo,&param);
        if(ret<0)perror("sched_setscheduler: ");
        cur_sched_algo = sched_getscheduler(pid);

        switch (pre_sched_algo){
            case 0:
                printf("pid=%d: pre scheduler: SCHED_NORMAL\n",pid);
            break;
            case 1:
                printf("pid=%d: pre scheduler: SCHED_FIFO\n",pid);
            break;
            case 2:
                printf("pid=%d: pre scheduler: SCHED_RR\n",pid);
            break;
            case 3:
                printf("pid=%d: pre scheduler: SCHED_BATCH\n",pid);
            break;
            case 6:
                printf("pid=%d: pre scheduler: SCHED_RAS\n",pid);
            break;
            default:
            break;
        }
        switch (cur_sched_algo){
            case 0:
                printf("pid=%d: cur scheduler: SCHED_NORMAL\n",pid);
            break;
            case 1:
                printf("pid=%d: cur scheduler: SCHED_FIFO\n",pid);
            break;
            case 2:
                printf("pid=%d: cur scheduler: SCHED_RR\n",pid);
            break;
            case 3:
                printf("pid=%d: cur scheduler: SCHED_BATCH\n",pid);
            break;
            case 6:
                printf("pid=%d: cur scheduler: SCHED_RAS\n",pid);
            break;
            default:
            break;
        }

        break;

    case 2:

        printf("please input which process you want to trace: pid = ");
        scanf("%d",&pid);
        sched_rr_get_interval(pid,&t);
        printf("Time slice of pid %d is %ld msec(s).\n",pid,t.tv_nsec/1000000);
        break;

    case 3:
        printf("please input which process you want to trace: pid = ");
        scanf("%d",&pid);
        printf("please input wcounts you want to set: wcounts = ");
        scanf("%d",&wcounts);
        syscall(365,pid,wcounts);
        printf("change wcounts finish!\n");

    default:
        break;
    }
    
    return 0;
}