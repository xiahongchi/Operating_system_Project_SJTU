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
/*
// The semaphore buffer's idea comes from "csapp"
typedef struct sbuf
{
    int *buf;
    int n;
    int front;
    int rear;
    sem_t mutex;
    sem_t slots;
    sem_t items;
} sbuf_t;
*/
void *Calloc(unsigned int num, unsigned int size);
/*
void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit();
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);
*/
void Listen(int __fd, int __n);
int Accept(int __fd, __SOCKADDR_ARG __addr,
           socklen_t *__addr_len);
/*
int Sem_init(sem_t *__sem, int __pshared, unsigned int __value);
int Sem_destroy(sem_t *__sem);
int Sem_wait(sem_t *__sem);
int Sem_post(sem_t *__sem);
*/

void Listen(int __fd, int __n)
{
    if (listen(__fd, __n) < 0)
    {
        fprintf(stderr, "ERROR on listen()");
        exit(1);
    }
}

int Accept(int __fd, __SOCKADDR_ARG __addr,
           socklen_t *__addr_len)
{
    int connfd = accept(__fd, __addr,
                        __addr_len);
    if (connfd < 0)
    {
        fprintf(stderr, "ERROR on accept()");
        exit(1);
    }
    return connfd;
}
/*
int Sem_init(sem_t *__sem, int __pshared, unsigned int __value)
{
    if (sem_init(__sem, __pshared, __value) < 0)
    {
        fprintf(stderr, "ERROR on sem_init()");
        exit(1);
    }
}

int Sem_wait(sem_t *__sem)
{
    if (sem_wait(__sem) < 0)
    {
        fprintf(stderr, "ERROR on sem_wait()");
        exit(1);
    }
}
int Sem_post(sem_t *__sem)
{
    if (sem_post(__sem) < 0)
    {
        fprintf(stderr, "ERROR on sem_post()");
        exit(1);
    }
}

void sbuf_init(sbuf_t *sp, int n)
{
    sp->buf = Calloc(n, sizeof(int));
    sp->n = n;
    sp->front = sp->rear = 0;
    sem_init(&sp->mutex, 0, 1);
    sem_init(&sp->slots, 0, n);
    sem_init(&sp->items, 0, 0);
}

void sbuf_deinit(sbuf_t *sp)
{
    free(sp->buf);
    sem_destroy(&sp->mutex);
    sem_destroy(&sp->items);
    sem_destroy(&sp->slots);
}

void sbuf_insert(sbuf_t *sp, int item)
{
    Sem_wait(&sp->slots);
    Sem_wait(&sp->mutex);
    sp->buf[(++sp->rear) % (sp->n)] = item;
    Sem_post(&sp->mutex);
    Sem_post(&sp->items);
}

int sbuf_remove(sbuf_t *sp)
{
    int item;
    Sem_wait(&sp->items);
    Sem_wait(&sp->mutex);
    item = sp->buf[(++sp->front) % (sp->n)];
    Sem_post(&sp->mutex);
    Sem_post(&sp->slots);
    return item;
}
*/
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