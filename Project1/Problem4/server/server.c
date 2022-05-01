#include "server.h"
#include "general.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int nserv = 0;

void *serve(void *newsockfd);

int main(int argc, char *argv[])
{
    /* Code section largely copied from WuFan's PPT */
    /* Make some modifications in several lines *
    /*---------------start------------------*/
    int sockfd, newsockfd, portno, clilen, n;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t ctid;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "ERROR opening socket\n");
        exit(1);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // the port number assigned
    portno = PORT;
    serv_addr.sin_port = htons(portno);
    // server ip address
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "Error on binding\n");
        exit(1);
    }
    Listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    printf("Server initiating...\n");
    /*----------------end-------------------*/
    /*--My Own Concurrent Multi-Thread Service--*/

    while (1)
    {
        int *pfd;
        pthread_t tid;
        newsockfd = accept(sockfd,
                           (struct sockaddr *)&cli_addr,
                           &clilen);
        if (newsockfd < 0)
        {
            fprintf(stderr, "error in accept()\n!");
            continue;
        }
        pfd = (int *)malloc(sizeof(int));
        *pfd = newsockfd;
        Pthread_create(&tid, NULL, serve, (void *)pfd);
        // Don't need pthread_join because thread detach itself
    }
    Close(sockfd);
    return 0;
}

void *serve(void *sockfd)
{
    char buffer[MAXBUFFER], line[MAXLINE];
    char *bptr;
    int cnt;
    int newsockfd;
    int start = 0;
    newsockfd = (int)(*((int *)sockfd));
    free(sockfd);

    // detach itself so we don't need pthread_join to recycle its resources
    Pthread_detach(pthread_self());

    /*My Encryption Here*/
    cnt = 0;
    bptr = buffer;

    while (read_line(buffer, &cnt, &bptr, newsockfd, line, MAXLINE) > 0)
    {
        if (start == 0)
        {
            pthread_mutex_lock(&mutex);
            if (nserv < MAXCLIBUF)
            {
                start = 1;
                nserv++;
            }
            pthread_mutex_unlock(&mutex);
        }
        if (start == 1)
        {
            if (strcmp(":q\n", line) == 0)
            {
                break;
            }
            int idx;
            printf("Receiving message: %s", line);
            for (idx = 0; line[idx]; idx++)
            {
                /* ---encrption here--- */
                if (line[idx] >= 'a' && line[idx] <= 'z')
                {
                    line[idx] = (line[idx] - 'a' + CAESAR_SHIFT) % 26 + 'a';
                }
                else if (line[idx] >= 'A' && line[idx] <= 'Z')
                {
                    line[idx] = (line[idx] - 'A' + CAESAR_SHIFT) % 26 + 'A';
                }
            }
            Write(newsockfd, line, idx);
        }
        else
        {
            int idx;
            strcpy(line, "Please wait...\n");
            for (idx = 0; line[idx]; idx++)
                ;
            Write(newsockfd, line, idx);
        }
    }
    printf("Server thread closing...\n");
    pthread_mutex_lock(&mutex);
    nserv--;
    pthread_mutex_unlock(&mutex);
    Close(newsockfd);
}
