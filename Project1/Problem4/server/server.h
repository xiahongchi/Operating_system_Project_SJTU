#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>

#define PORT 2050

#define MAXCLISEV 2
#define MAXCLIBUF 2
#define CAESAR_SHIFT 3

void *Calloc(unsigned int num, unsigned int size);

void Listen(int __fd, int __n);

void Listen(int __fd, int __n)
{
    if (listen(__fd, __n) < 0)
    {
        fprintf(stderr, "ERROR on listen()");
        exit(1);
    }
}

void *Calloc(unsigned int num, unsigned int size)
{
    void *p = calloc(num, size);
    if (p == NULL)
    {
        fprintf(stderr, "ERROR on calloc()");
        exit(1);
    }
    return p;
}

#endif