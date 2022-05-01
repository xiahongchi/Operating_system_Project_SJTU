#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "general.h"
#define PORT 2050

int main(int argc, char *argv[])
{
    /* Code section largely copied from WuFan's PPT */
    /* Make some modifications in several lines *
    /*---------------start------------------*/
    int sockfd = 0, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[MAXBUFFER];
    // the port of the server
    portno = PORT;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "ERROR opening socket");
        exit(1);
    }
    server = gethostbyname("127.0.0.1");
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host");
        exit(1);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "ERROR connecting");
        exit(1);
    }
    printf("Please enter the message:\n");
    /*----------------end-------------------*/
    /*-------My Own Client Program Here-------*/
    while (1)
    {
        int k = 0, rs;
        char receive[MAXBUFFER];
        bzero((void *)buffer, MAXBUFFER);
        bzero((void *)receive, MAXBUFFER);
        fgets(buffer, MAXBUFFER, stdin);
        buffer[MAXBUFFER - 1] = '\0';
        buffer[MAXBUFFER - 2] = '\n';
        Write(sockfd, buffer, strlen(buffer));
        rs = read(sockfd, receive, MAXBUFFER);
        if (rs < 0)
        {
            fprintf(stderr, "read error!");
            exit(1);
        }
        else if (rs == 0)
        {
            break;
        }
        else
        {
            printf("From server: %s", receive);
        }
    }
    printf("Client closing...\n");
    return 0;
}