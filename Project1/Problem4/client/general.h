#ifndef __GENERAL_H__
#define __GENERAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <errno.h>

#define MAXBUFFER 256
#define MAXLINE 256

typedef void *(func)(void *);
void Close(int __fd);
ssize_t Write(int __fd, const void *__buf, size_t __n);
void Pthread_create(pthread_t *tid, pthread_attr_t *attr,
                    func *f, void *arg);

// The next two functions' idea comes from "csapp"
int read_from_buf(char buffer[], int *cnt, char **bptr,
                  int fd, char *sc, unsigned n);
unsigned read_line(char buffer[], int *cnt, char **bptr,
                   int fd, void *sc, unsigned length);

int read_from_buf(char buffer[], int *cnt, char **bptr,
                  int fd, char *sc, unsigned n)
{
    int c;
    while (*cnt <= 0)
    {
        *cnt = read(fd, buffer, MAXBUFFER);
        if (*cnt < 0)
        {
            if (errno != EINTR)
                return -1;
        }
        else if (*cnt == 0)
            return 0;
        else
            *bptr = buffer;
    }
    c = n;
    if (*cnt < n)
        c = *cnt;
    memcpy(sc, *bptr, c);
    *bptr = *bptr + c;
    *cnt = *cnt - c;
    return c;
}

unsigned read_line(char buffer[], int *cnt, char **bptr,
                   int fd, void *sc, unsigned length)
{
    int n, rc;
    char c, *bufp = sc;
    for (n = 1; n < length; n++)
    {
        rc = read_from_buf(buffer, cnt, bptr, fd, &c, 1);
        if (rc == 1)
        {
            *bufp++ = c;
            if (c == '\n')
            {
                n++;
                break;
            }
        }
        else if (rc == 0)
        {
            if (n == 1)
                return 0;
            else
                break;
        }
        else
            return -1;
    }
    *bufp = 0;
    return n - 1;
}

void Close(int __fd)
{
    if (close(__fd) < 0)
    {
        fprintf(stderr, "ERROR on close()");
        exit(1);
    }
}

ssize_t Write(int __fd, const void *__buf, size_t __n)
{
    int rs = write(__fd, __buf, __n);
    if (rs < 0)
    {
        fprintf(stderr, "ERROR on write()");
        exit(1);
    }
    return rs;
}

void Pthread_create(pthread_t *tid, pthread_attr_t *attr,
                    func *f, void *arg)
{
    if (pthread_create(tid, attr, f, arg) != 0)
    {
        fprintf(stderr, "ERROR on pthread_create()");
        exit(1);
    }
}

void Pthread_detach(pthread_t __th)
{
    if (pthread_detach(__th) < 0)
    {
        fprintf(stderr, "ERROR on pthread_detach()");
        exit(1);
    }
}
#endif
