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

char *find_param(char **tokens, char *param, int size)
{
    char *res = NULL;
    for (int i = 0; i < size; i++)
    {
        res = strstr(tokens[i], param);
        if (res)
            break;
    }

    return res;
}
// Parse response from server
void *parse_message(void *arg)
{
    int *fd = arg;
    ssize_t n;

    char *receive = xcalloc(DEFAULT_BUFFER_SIZE, sizeof(char));
    while (42)
    {
        // test
        receive = xrealloc(receive, DEFAULT_BUFFER_SIZE * sizeof(char));
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
                    if (strcmp(tokens[0], "0") != 0)
                    {
                        write(1, "< ", strlen("< "));
                        write(1, payload, atoi(tokens[0]));
                    }
                    break;
                case 2:
                    if (strcmp(tokens[2], "SEND-DM") == 0)
                    {
                        char *param = find_param(tokens, "From=", count);
                        param[4] = ' ';
                        // Utiliser write
                        char *res = NULL;
                        size_t len = asprintf(&res, "%s: %s\n", param, payload);
                        write(1, res, len);
                    }
                    if (strcmp(tokens[2], "BROADCAST") == 0)
                    {
                        char *param = find_param(tokens, "From=", count);
                        param[4] = ' ';
                        char *res = NULL;
                        size_t len = asprintf(&res, "%s: %s\n", param, payload);
                        write(1, res, len);
                    }
                    if (strcmp(tokens[2], "SEND-ROOM") == 0)
                    {
                        char *param1 = find_param(tokens, "From=", count);
                        param1[4] = ' ';
                        char *param2 = find_param(tokens, "Room=", count);
                        param2 += 5;
                        char *res = NULL;
                        size_t len = asprintf(&res, "%s@%s: %s\n", param1,
                                              param2, payload);
                        write(1, res, len);
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
/*
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
}*/

// My getline.
char *my_getline()
{
    char *lineptr = xcalloc(DEFAULT_BUFFER_SIZE, sizeof(char));
    size_t size = DEFAULT_BUFFER_SIZE;

    int c;
    size_t n = 0;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        lineptr[n] = c;
        n++;

        if (n >= size)
        {
            lineptr = xrealloc(lineptr, size * 2);
            size *= 2;
        }
    }

    if (n == 0)
    {
        if (c == '\n')
        {
            lineptr[0] = '\n';
        }
        else
        {
            free(lineptr);
            lineptr = NULL;
        }
    }
    else
    {
        lineptr[n] = '\0';
    }

    return lineptr;
}

// Check if str is in tab
int is_in(char *str, char **tab, size_t size)
{
    size_t i = 0;
    while (i < size && strcmp(str, tab[i]) != 0)
    {
        i++;
    }
    if (i == size)
        return 1;
    return 0;
}

// Checks parameter validity. Parameter must contain new line..
int is_valid_param(char *param)
{
    char *r = strstr(param, "=");

    if (r && (param[0] != '=') && (param[strlen(param) - 1] != '='))
        return 1;

    return 0;
}

void get_params(struct params_payload *p)
{
    int timeout = 2;
    char *lineptr = NULL;

    while (timeout)
    {
        lineptr = xcalloc(DEFAULT_BUFFER_SIZE, sizeof(char));
        size_t size = DEFAULT_BUFFER_SIZE;

        int c;
        size_t n = 0;
        while ((c = getchar()) != '\n' && c != EOF)
        {
            lineptr[n] = c;
            n++;

            if (n >= size)
            {
                lineptr = xrealloc(lineptr, size * 2);
                size *= 2;
            }
        }

        if (n == 0)
        {
            timeout--;
        }
        else
        {
            // lineptr[n] = '\0';
            if (is_valid_param(lineptr) == 0)
            {
                fprintf(stderr, "Invalid parameter\n");
                // fflush(sttderr)
            }
            else
            {
                char *r = strstr(lineptr, "=");

                if (r)
                {
                    r[strlen(r) - 1] = '\0';
                    p->params = add_param(p->params, lineptr, NULL);
                }
            }
        }

        if (lineptr)
        {
            free(lineptr);
            lineptr = NULL;
        }

        // fflush(stdin);
    }

    if (lineptr)
    {
        free(lineptr);
        lineptr = NULL;
    }
}

void get_payload(struct params_payload *params, char *command,
                 int server_socket, char *send)
{
    char *payload = NULL;

    while (fprintf(stdout, "Payload:\n") && (payload = my_getline()) != NULL)
    {
        if (strcmp(payload, "/quit") == 0)
        {
            // fflush(stdin);
            break;
        }

        if (payload[0] != '\n')
            asprintf(&params->payload, "%s", payload);
        else
            payload[0] = '\0';

        send = gen_message(strlen(payload), 0, command, params);
        resend(send, strlen(send), server_socket);
        free(send);
        send = NULL;

        // Clear stdin buffer.
        // fflush(stdin);
    }

    if (payload)
        free(payload);
}

// If no newline in param, skip don't do anything (or just stop looking
// for param).
// TODO Find out what case for parameters is valid but returning false
// in code.
void communicate(int server_socket)
{
    char *send = NULL;
    char *command = NULL;
    char *commands[10] = { "PING",        "LIST-USERS", "LIST-ROOMS",
                           "PROFILE",     "LOGIN",      "BROADCAST",
                           "CREATE-ROOM", "JOIN-ROOM",  "LEAVE-ROOM",
                           "DELETE-ROOM" };
    char *args_commands[2] = { "SEND-DM", "SEND-ROOM" };

    // Use while 1 instead then do parsing to prevent eof check.
    while (fprintf(stdout, "Command:\n") && (command = my_getline()) != NULL)
    {
        struct params_payload *params =
            xcalloc(1, sizeof(struct params_payload));

        // fflush(stdin);

        if (is_in(command, args_commands, 2) == 0)
        {
            fprintf(stdout, "Parameters:\n");
            get_params(params);
            get_payload(params, command, server_socket, send);
        }
        else if (is_in(command, commands, 10) == 0)
        {
            // TODO Use different payload (so make code cleaner).
            fprintf(stdout, "Payload:\n");
            char *payload = my_getline();

            if (payload)
            {
                if (payload[0] != '\n')
                    asprintf(&params->payload, "%s", payload);
                else
                    payload[0] = '\0';

                send = gen_message(strlen(payload), 0, command, params);
                free(payload);
            }
            // fflush(stdin);
        }
        else
        {
            fprintf(stderr, "Invalid command\n");
            // fflush(stdin);
        }

        if (send)
        {
            resend(send, strlen(send), server_socket);
            free(send);
            send = NULL;
        }

        // Free elements.
        if (command)
            free(command);

        if (params)
            free_payload(params);
    }
}

/*
int main(void)
{
    struct params_payload *p = xcalloc(1,sizeof(struct params_payload));

    p->params = add_param(p->params, "User", "Robert");
    p->params = add_param(p->params, "From", "Clarel");
    p->params = add_param(p->params, "Big", "Drip");
    size_t size = asprintf(&p->payload, "Fre sh avacado");

    char *message = gen_message(size, 0, "SEND-DM", p);

    write(1, message, strlen(message));

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
