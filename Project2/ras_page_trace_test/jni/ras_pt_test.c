/*
 * extend the base test program mem_visit.c
 */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>
 
static int alloc_size;
static char* memory;
static int times;

#define TEST_PROC 10
#define SCHED_RAS 6
void segv_handler(int signal_number)
{
	mprotect(memory, alloc_size, PROT_READ | PROT_WRITE);
	times++;
}

void test_ras(int proc,int fd);

 
int main()
{
	int fd;
	struct sigaction sa;
    
    int proc;
    pid_t pid;

    struct sched_param param;
    int sched_algo,sched_prio,pre_sched_algo,cur_sched_algo;

	/* Init segv_handler to handle SIGSEGV */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &segv_handler;
	sigaction(SIGSEGV, &sa, NULL);



    param.sched_priority=0;
    printf("Start ras memory trace testing program!\n");

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

    times = 0;

	/* allocate memory for process, set the memory can only be read */
	alloc_size = 32 * getpagesize();
	fd = open("/dev/zero", O_RDONLY);
	memory = mmap(NULL, alloc_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    /* fork children processes and will share the same open file space */
    for(proc=0;proc<TEST_PROC;proc++){
        pid=fork();
        if(pid<0) exit(-1);
        if(pid==0){
            printf("spawning process: pid=%d, proc=%d\n",getpid(),proc);
            pre_sched_algo = sched_getscheduler(getpid());
            sched_setscheduler(getpid(),sched_algo,&param);
            cur_sched_algo = sched_getscheduler(getpid());
            switch (pre_sched_algo){
                case 0:
                    printf("pid=%d: pre scheduler: SCHED_NORMAL\n",getpid());
                break;
                case 1:
                    printf("pid=%d: pre scheduler: SCHED_FIFO\n",getpid());
                break;
                case 2:
                    printf("pid=%d: pre scheduler: SCHED_RR\n",getpid());
                break;
                case 3:
                    printf("pid=%d: pre scheduler: SCHED_BATCH\n",getpid());
                break;
                case 6:
                    printf("pid=%d: pre scheduler: SCHED_RAS\n",getpid());
                break;
                default:
                break;
            }
            switch (cur_sched_algo){
                case 0:
                    printf("pid=%d: cur scheduler: SCHED_NORMAL\n",getpid());
                break;
                case 1:
                    printf("pid=%d: cur scheduler: SCHED_FIFO\n",getpid());
                break;
                case 2:
                    printf("pid=%d: cur scheduler: SCHED_RR\n",getpid());
                break;
                case 3:
                    printf("pid=%d: cur scheduler: SCHED_BATCH\n",getpid());
                break;
                case 6:
                    printf("pid=%d: cur scheduler: SCHED_RAS\n",getpid());
                break;
                default:
                break;
            }
            /* tracing rr or fifo */
            syscall(364,getpid());
            sleep(3); //give some time to let all child processes fork and then test them
            test_ras(proc,fd);
            //after test then sleep for some time for detecting
            exit(0);
        }
    }


    for(proc=0;proc<TEST_PROC;proc++){
        wait(NULL);
    }

	close(fd);

	/* free */
	munmap(memory, alloc_size);

	return 0;
}

void test_ras(int proc,int fd){
    
    int visit_times;
    unsigned wcount;

    /* syscall page_start_trace */
	syscall(361, getpid(),memory,alloc_size,PROT_READ);

    close(fd);

    for(visit_times=0;visit_times<(1<<(proc+1));visit_times++){
        /* try to write */
        memory[visit_times] = 0;
        /* write finish and then set protection */
	    mprotect(memory, alloc_size, PROT_READ);
    }

    for(visit_times=0;visit_times<(1<<(proc+1));visit_times++){
        /* try to write */
        memory[visit_times*2+proc] = 0; //AT DIFFERENT MEMORY RANGE
        /* write finish and then set protection */
	    mprotect(memory, alloc_size, PROT_READ);
    }

    for(visit_times=0;visit_times<(1<<(proc+1));visit_times++){
        /* try to write */
        memory[visit_times*4+proc] = 0; //AT DIFFERENT MEMORY RANGE
        /* write finish and then set protection */
	    mprotect(memory, alloc_size, PROT_READ);
    }

    for(visit_times=0;visit_times<(1<<(proc+1));visit_times++){
        /* try to write */
        memory[visit_times*6+proc] = 0; //AT DIFFERENT MEMORY RANGE
        /* write finish and then set protection */
	    mprotect(memory, alloc_size, PROT_READ);
    }

    /* syscall page_stop_trace */
	syscall(362, getpid());

    /* Get wcount */
    syscall(363, getpid(), &wcount);
    printf("task pid : %d, proc number: %d, wcount = %u, times = %d\n", 
        getpid(), proc, wcount, times);

}
