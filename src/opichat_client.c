#define _GNU_SOURCE
#include "opichat_client.h"

#include <err.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

// TODO Shorten code.
// Creates tokens from parsed message and its count.
char **lexer(char *receive, int *tokens_count)
{
    *tokens_count = 3;
    char **tokens = xcalloc(*tokens_count, sizeof(char *));

    char *token = NULL;
    char *save = NULL;

    token = strtok_r(receive, "\n", &save);
    asprintf(&tokens[0], "%s", token);

    // Get payload before strtok use.
    char *payload = NULL;

    if (strcmp(tokens[0], "0") != 0)
    {
        payload = strstr(save, "\n\n");
        char *payload_cpy = payload;
        payload += 2;
        asprintf(&payload, "%s", payload);

        // Null terminate begining of payload in received data.
        *payload_cpy = '\0';
    }

    // Save payload size, status and command.
    for (int i = 1; i < 3; i++)
    {
        token = strtok_r(NULL, "\n", &save);
        asprintf(&tokens[i], "%s", token);
    }

    // Get data parameters.
    token = strtok_r(NULL, "\n", &save);

    while (token)
    {
        *tokens_count += 1;
        tokens = xrealloc(tokens, *tokens_count * sizeof(char *));
        asprintf(&tokens[*tokens_count - 1], "%s", token);

        token = strtok_r(NULL, "\n", &save);
    }

    // Add delimiter after parameters.
    *tokens_count += 1;
    tokens = xrealloc(tokens, *tokens_count * sizeof(char *));
    asprintf(&tokens[*tokens_count - 1], "%s", "");

    if (payload)
    {
        size_t payload_size = atoll(tokens[0]);
        *tokens_count += 1;

        tokens = xrealloc(tokens, *(tokens_count) * sizeof(char *));
        tokens[*tokens_count - 1] = xcalloc(payload_size + 1, sizeof(char));
        memcpy(tokens[*tokens_count - 1], payload, payload_size);
    }

    return tokens;
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
            // Parsing message(s)
            int count = 0;
            char **tokens = lexer(receive, &count);

            switch (atoi(tokens[1]))
            {
            case 1:
                if (tokens[0] != 0)
                {
                    puts("< ");
                    write(1, tokens[count - 1], atoi(tokens[0]));
                }
                break;
            case 2:
                break;
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
    while (fprintf(stderr, "Enter your message:\n")
           && (res = getline(&lineptr, &n, stdin)) != -1)
    {
        resend(lineptr, res, server_socket);
    }
    if (lineptr != NULL)
        free(lineptr);
}

int main(int argc, char **argv)
{
    if (argc != 3)
        errx(1, "usage: ./opichat_client <ip> <port>");

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
