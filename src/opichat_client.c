#define _GNU_SOURCE
#include "opichat_client.h"
#include "utils/xalloc.h"

#include <err.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

// Creates tokens from message received from server.
char **lexer(char *receive)
{
    size_t tokens_count = 3;
    char **tokens = xcalloc(tokens_count, sizeof(char *));
    // ['4', '1', 'commande', 'par1', 'par2'..., NULL, 'payload']

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
    puts(token);
    while (token)
    {
        tokens_count++;
        tokens = xrealloc(tokens, tokens_count * sizeof(char *));
        asprintf(&tokens[tokens_count], "%s", token);

        token = strtok_r(NULL, "\n", &save);
    }

    // Add delimiter after parameters.
    tokens[tokens_count - 1] = NULL;

    if (payload)
    {
        tokens = xrealloc(tokens, tokens_count + 1);
        memcpy(tokens[tokens_count], payload, atoll(tokens[0]));
    }

    return tokens;
}

// Parse response from server
void *parse_message(void *arg)
{
    int *fd = arg;
    char *receive = xmalloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
    ssize_t n;
    ssize_t i = 0;

    printf("Server answered with: ");
    fflush(stdout);

    while (42)
    {
        while ((n = recv(*fd, receive + i, DEFAULT_BUFFER_SIZE, 0)) != -1)
        {
            // Realloc the size
            i += n;
            receive = xrealloc(receive, i);
        }
        //Parsing message(s)
        //

        /*
        switch (status)
        {
            case 1:
                if (payload_size != 0)
                {
                    '<'
                }
                break;
            case 2:

        }
        */
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

int main(void)
{
    // Copy only payoad_size data to payload side.
    char test[] = "9\n2\nSEND-DM\nUser=acu\nFrom=ING1\n\n2022\n2021";

    char **tokens = lexer(test);

    for (int i = 0; i < 5; i++)
    {
        puts(tokens[i]);
    }
}
/*
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
    pthread_create(&id, NULL, &print_response, arg);

    communicate(server_socket);

    return 0;
}
*/
