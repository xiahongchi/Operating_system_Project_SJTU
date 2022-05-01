#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int alloc_size;
static char* memory;
char* heap_memory;
int heap_alloc_sice;

static int times;

#define proc 8

void segv_handler(int signal_number) {
    // printf("find memory accessed!\n");
    mprotect(memory, alloc_size, PROT_READ | PROT_WRITE);
    mprotect(heap_memory, heap_alloc_sice, PROT_READ | PROT_WRITE);
    times++;
    // printf("set memory read write!\n");
}

int main() {
    int fd;
    struct sigaction sa;
    unsigned wcount;
    int visit_times;

    printf("Start memory trace testing program!\n");

    /* Init segv_handler to handle SIGSEGV */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &segv_handler;
    sigaction(SIGSEGV, &sa, NULL);

    times = 0;

    /* allocate memory for process, set the memory can only be read */
    alloc_size = 32 * getpagesize();
    fd = open("/dev/zero", O_RDONLY);
    memory = mmap(NULL, alloc_size, PROT_READ, MAP_PRIVATE, fd, 0);

    /* allocate another memory space in heap for test */
    heap_memory = (char*)malloc(1 * getpagesize());
    heap_alloc_sice = 1 * getpagesize();

    /* system call of page_start_trace() */
    syscall(361, getpid(), memory, alloc_size, PROT_READ);
    close(fd);

    printf("Repeat the test for different virtual address range\n");

    printf("First test: \n");
    for (visit_times = 0; visit_times < (1 << (proc + 1)); visit_times++) {
        /* try to write */
        memory[visit_times] = 0;
        /* write finish and then set protection */
        mprotect(memory, alloc_size, PROT_READ);
    }

    /* Get wcount */
    syscall(363, getpid(), &wcount);
    printf(
        "After first test: task pid : %d, wcount = %u, times "
        "= %d\n",
        getpid(), wcount, times);

    printf("Second test:\n");
    for (visit_times = 0; visit_times < (1 << (proc + 1)); visit_times++) {
        /* try to write */
        memory[visit_times * 2 + proc] = 0;  // AT DIFFERENT MEMORY RANGE
        /* write finish and then set protection */
        mprotect(memory, alloc_size, PROT_READ);
    }

    /* Get wcount */
    syscall(363, getpid(), &wcount);
    printf(
        "After second test: task pid : %d, wcount = %u, times "
        "= %d\n",
        getpid(), wcount, times);

    printf(
        "Repeat the test for different time intervals: sleep for one "
        "second...\n");
    sleep(1);
    printf("Wake up! Now the third test:\n");
    for (visit_times = 0; visit_times < (1 << (proc + 1)); visit_times++) {
        /* try to write */
        memory[visit_times * 4 + proc] = 0;  // AT DIFFERENT MEMORY RANGE
        /* write finish and then set protection */
        mprotect(memory, alloc_size, PROT_READ);
    }

    /* Get wcount */
    syscall(363, getpid(), &wcount);
    printf(
        "After third test: task pid : %d, wcount = %u, times "
        "= %d\n",
        getpid(), wcount, times);

    printf(
        "After testing three times in one large allocated memory place with "
        "different address, now change to another memory place:\n");

    for (visit_times = 0; visit_times < (1 << (proc + 1)); visit_times++) {
        /*set protection */
        mprotect(heap_memory, heap_alloc_sice, PROT_READ);
        /* try to write */
        heap_memory[visit_times] = 0;  // AT DIFFERENT MEMORY RANGE
    }

    /* Get wcount */
    syscall(363, getpid(), &wcount);
    printf(
        "After fourth test: task pid : %d, wcount = %u, times "
        "= %d\n",
        getpid(), wcount, times);

    /* syscall page_stop_trace */
    syscall(362, getpid());

    printf("Test finished!\n");

    /* free */
    free(heap_memory);
    munmap(memory, alloc_size);
    return 0;
}