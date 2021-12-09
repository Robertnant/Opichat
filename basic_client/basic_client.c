#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "basic_client.h"

int create_and_connect(struct addrinfo *addrinfo)
{
    int sfd;
    struct addrinfo *rp = NULL;
    for (rp = addrinfo; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;
        close(sfd);
    }
    if (rp == NULL)
    {
        err(1, "basic_client: connection failed");
    }
    freeaddrinfo(addrinfo);
    return sfd;
}

int prepare_socket(const char *ip, const char *port)
{
    struct addrinfo *addrinfo;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int value = getaddrinfo(ip, port, &hints, &addrinfo);
    if (value != 0)
        err(value, "basic_client: cannot get addresses");

    return create_and_connect(addrinfo);
}

// Handle long messages by sending in file descriptor
void resend(const char *buff, size_t len, int fd)
{
    size_t l = 0;
    while (l < len)
    {
        ssize_t res = send(fd, buff + l, len - l, 0);
        if (res == -1)
        {
            err(1, "basic_client: failed to send data");
        }
        l += res;
    }
}

// Reads response from server and prints to sdout
void print_response(int fd)
{
    char receive[DEFAULT_BUFFER_SIZE];
    ssize_t n;
    printf("Server answered with: ");
    while ((n = recv(fd, receive, DEFAULT_BUFFER_SIZE, 0) != 0))
    {
        if (n == -1)
            err(1, "basic_client: failed to receive response");

        resend(receive, n, 1);
    }
}


void communicate(int server_socket)
{
    ssize_t res;
    char *lineptr = NULL;
    size_t n = 0;
    while (fprintf(stderr, "Enter your message:\n") &&
            (res = getline(&lineptr, &n, stdin)) != -1)
    {
        resend(lineptr, res, server_socket);
        print_response(server_socket);

        //free memory and reset n
        /*free(lineptr);
        lineptr = NULL;
        n = 0;*/
    }
    if (lineptr != NULL)
        free(lineptr);
}

