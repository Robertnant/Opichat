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
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable,
                    sizeof(int)) == -1)
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

    // Enable 5 incoming connections (5 by default).
    if ((listen(sfd, 5)) == -1)
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

// Reads response from client and prints to sdout then back to client
void communicate(int client_socket)
{
    char receive[DEFAULT_BUFFER_SIZE];
    ssize_t n;

    fflush(stdout);

    while (printf("Received Body: ") &&
            (n = recv(client_socket, receive, DEFAULT_BUFFER_SIZE, 0)) != -1)
    {
        // Move to next client when nothing is received.
        if (n == 0)
        {
            fflush(stdout);
            puts("Client disconnected");
            //fflush(stdout);
            break;
        }

        // Prints client response to standard output till newline reached.
        // Sends the same message back to client.
        ssize_t l_write = 0;
        ssize_t l_send = 0;
        ssize_t res = 0;

        while (l_write < n || l_send < n)
        {
            if (l_send < n)
            {
                res = send(client_socket,
                        receive + l_send, n - l_send, MSG_NOSIGNAL);

                if (errno == EPIPE)
                {
                    break;
                }

                if (res == -1)
                    err(1, "failed to send data back to client");

                l_send += res;
            }

            if (l_write < n)
            {
                res = write(1, receive + l_write, n - l_write);

                if (errno == EPIPE)
                {
                    break;
                }

                if (res == -1)
                {
                    err(1, "failed to write data to stdout");
                }

                l_write += res;
            }
        }
    }
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
