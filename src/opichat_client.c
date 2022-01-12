#define _GNU_SOURCE

#include "opichat_client.h"

#include <err.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils/lexer.h"
#include "utils/xalloc.h"

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
        err(1, "connection failed");
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
        err(value, "cannot get addresses");

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
            err(1, "failed to send data");
        }
        l += res;
    }
}

// Parse response from server
void *parse_message(void *arg)
{
    int *fd = arg;
    char *receive = xcalloc(DEFAULT_BUFFER_SIZE, sizeof(char));
    ssize_t n;

    while (42)
    {
        ssize_t i = 0;
        while ((n = recv(*fd, receive + i, DEFAULT_BUFFER_SIZE, 0)) != -1)
        {
            // Realloc the size and add one byte for null termination.
            i += n;
            receive = xrealloc(receive, (i + 1) * sizeof(char));
        }

        if (i != 0)
        {
            receive[i] = '\0';
            char *next_message = receive;

            // Parsing message(s)
            while (next_message[0])
            {
                int count = 0;
                char **tokens = lexer(&next_message, &count);

                char *payload = tokens[count - 1];
                switch (atoi(tokens[1]))
                {
                case 1:
                    if (tokens[0] != 0)
                    {
                        write(1, "< ", strlen("< "));
                        write(1, payload, atoi(tokens[0]));
                    }
                    break;
                case 2:
                    if (strcmp(tokens[2], "SEND-DM") == 0)
                    {
                        printf("From %s: %s\n", tokens[4], payload);
                    }
                    break;
                case 3:
                    write(2, "! ", strlen("! "));
                    write(2, payload, atoi(tokens[0]));
                    break;
                }
            }
        }
    }
    return NULL;
}

void communicate(int server_socket)
{
    ssize_t res;
    char *lineptr = NULL;
    size_t n = 0;
    while ((res = getline(&lineptr, &n, stdin)) != -1)
    {
        resend(lineptr, res, server_socket);
    }
    if (lineptr != NULL)
        free(lineptr);
}

/*
int main(void)
{
    char *tmp = NULL;
    asprintf(&tmp, "13\n1\nDELETE-ROOM\n\nRoom Deleted\n");

    char *str = tmp;
    while (str[0])
    {
        printf("\nMessage: \n");

        int count = 0;
        char **tokens = lexer(&str, &count);

        for (int i = 0; i < count; i++)
        {
            printf("Got: %s | Done\n", tokens[i]);
            free(tokens[i]);
        }

        free(tokens);
    }

    free(tmp);
    tmp = NULL;
    return 0;
}
*/

int main(int argc, char **argv)
{
    if (argc != 3)
        errx(1, "usage: %s <ip> <port>", argv[0]);

    // Prepare socket and handle communication.
    int server_socket = prepare_socket(argv[1], argv[2]);

    // Make socket non blocking.
    fcntl(server_socket, F_SETFL, O_NONBLOCK);

    pthread_t id;
    void *arg = &server_socket;
    pthread_create(&id, NULL, &parse_message, arg);

    communicate(server_socket);

    return 0;
}
