#include "basic_server.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int create_and_bind(struct addrinfo *addrinfo)
{
    struct addrinfo *rp = NULL;
    int sfd;

    // Iterate through each address and attempt connection.
    for (rp = addrinfo; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (sfd == -1)
            continue;

        // Enable reuse of address.
        int enable = 1;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))
            == -1)
        {
            err(1, "socket option setting failed");
        }

        // Bind socket to address.
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;
    }

    // Check for bind success.
    if (rp == NULL)
        err(1, "could not bind any socket to address");

    return sfd;
}

int prepare_socket(const char *ip, const char *port)
{
    struct addrinfo *addrinfo;
    struct addrinfo hints;

    // Initialize hints fields.
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Get addresses.
    int value = getaddrinfo(ip, port, &hints, &addrinfo);
    if (value != 0)
        err(1, "failed to get address info");

    // Get binded socket.
    int sfd = create_and_bind(addrinfo);

    // Enable 1 incoming connection (1 by default).
    if ((listen(sfd, 1)) == -1)
        err(1, "listen failed");

    return sfd;
}

int accept_client(int socket)
{
    // Accept an incoming connection.
    struct sockaddr client;
    socklen_t connfd_len = sizeof(client);
    int connfd = accept(socket, &client, &connfd_len);

    if (connfd == -1)
        err(1, "failed to accept connection");

    puts("Client connected");

    return connfd;
}

// Handle long messages by sending in file adescriptor
// Returns 1 if client disconnection signaled.
int resend(const char *buff, size_t len, int fd)
{
    size_t l = 0;
    while (l < len)
    {
        ssize_t res = send(fd, buff + l, len - l, MSG_NOSIGNAL);

        if (errno == EPIPE)
            return 1;

        if (res == -1)
        {
            err(1, "failed to send data");
        }
        l += res;
    }

    return 0;
}

// Reads response from client and prints to sdout then back to client
void communicate(int client_socket)
{
    char *receive = NULL;
    ssize_t n;

    while (1)
    {
        ssize_t buffer_size = DEFAULT_BUFFER_SIZE;
        ssize_t total_len = 0;
        receive = calloc(buffer_size, sizeof(char));

        // Check if message contains newline before handling it.
        while ((n = recv(client_socket, receive + total_len,
                         DEFAULT_BUFFER_SIZE, 0)))
        {
            if (n == 0 || n == -1)
                break;

            total_len += n;

            // Reallocate space if necessary for next call.
            if (buffer_size <= total_len)
            {
                buffer_size += DEFAULT_BUFFER_SIZE;
                receive = realloc(receive, buffer_size);
            }

            // Send message back once newline received.
            if (receive[total_len - 1] == '\n')
            {
                receive[total_len] = '\0';
                break;
            }
        }

        if (n == 0 || n == -1)
            break;

        // Send message back and check client disconnection.
        if (resend(receive, total_len, client_socket) == 1)
            break;

        printf("Received body: %s", receive);
    }

    free(receive);
    puts("Client disconnected");
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        puts("Usage: ./basic_server SERVER_IP SERVER_PORT");
        return 1;
    }

    // Get listening socket.
    int listen_sock = prepare_socket(argv[1], argv[2]);

    // Wait for connections and start communication. 42 is equivalent to true.
    while (42)
    {
        int client_socket = accept_client(listen_sock);
        communicate(client_socket);

        // Close client connection.
        close(client_socket);
    }

    close(listen_sock);

    return 0;
}
