#define _GNU_SOURCE

#include "opichat_server.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "utils/xalloc.h"

/**
 * \brief Iterate over the struct addrinfo elements to create and bind a socket
 *
 * \param addrinfo: struct addrinfo elements
 *
 * \return The created socket or exit with 1 if there is an error
 *
 * Try to create and connect a socket with every addrinfo element until it
 * succeeds
 *
 */
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

/**
 * \brief Initialize the Addrinfo struct and call create_and bind() and
 * listen(2)
 *
 * \param ip: IP address of the server
 * \param port: Port of the server
 *
 * \return The created socket
 *
 * Initialize the struct addrinfo needed by create_and_bind() before calling
 * it. When create_and_bind() returns a valid socket, set the socket to
 * listening and return it.
 */
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

/**
 * \brief Accept a new client and add it to the connection_t struct
 *
 * \param epoll_instance: the epoll instance
 * \param server_socket: listening socket
 * \param connection: the connection linked list with all the current
 * connections
 *
 * \return The connection struct with the new client added
 */
struct connection_t *accept_client(int epoll_instance, int server_socket,
                                   struct connection_t *connection)
{
    // Accept an incoming connection.
    struct sockaddr client;
    socklen_t connfd_len = sizeof(client);
    int connfd = accept(server_socket, &client, &connfd_len);

    if (connfd == -1)
    {
        perror("failed to accept new client");
        close(connfd);
    }
    else
    {
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = connfd;

        if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, connfd, &event) == -1)
        {
            perror("failed to add client fd to epoll instance");
            close(connfd);
        }
        else
        {
            connection = add_client(connection, connfd);
            puts("Client connected");
        }
    }

    return connection;
}

// Handles long messages to send.
int resend(const char *buff, size_t len, int fd)
{
    size_t l = 0;
    while (l < len)
    {
        ssize_t res = send(fd, buff + l, len - l, 0);

        if (res == -1)
        {
            perror("failed to broadcast data to a client");
            return 1;
        }
        l += res;
    }

    return 0;
}

// Sends message to a client and handles errors.
void send_message(char *buffer, size_t len, int fd,
                  struct connection_t *connection)
{
    printf("< REQUEST_IN: %s", buffer);

    if (resend(buffer, len, fd) == 1)
    {
        printf("failed to send message to client %d", fd);
        connection = remove_client(connection, fd);
    }
}

// TODO
// Step 1: act as if server will only receive one client command at a time.

// Step 2: Handle invalid requests.

// Step 3: Handle case where multiple commands received (process commands till
// last newline found in buffer.
// Read Beej's guide in case of doubt.

struct connection_t *process_message(struct connection_t *client,
                                     struct connection_t *connection)
{
    // Get payload size. (use strtok_re with \n)
    char *token = NULL;
    token = strtok(client->buffer, "\n");

    // TODO use atol if atoi not good enough.
    ssize_t payload_size = atoi(token);

    // Check if full message received (payload_size, len(payload_size), status).
    // TODO Find way to handle size of parameters.
    if (client->nb_read < payload_size)
    {
        printf("Waiting for full message of fd: %d\n", client->client_socket);
        return connection;
    }

    // Get status code (one byte).
    token = strtok(NULL, "\n");
    int status = atoi(token);

    if (status != 0)
        puts("invalid request");

    // Use switch to determine which command was received.
    token = strtok(NULL, "\n");

    // TODO Find empty newline "\n\n" to get to payload
    // for messages with parameters.
    if (strcmp(token, "PING") == 0)
    {
        char *pong = "5\n1\nPING\n\nPONG\n";
        send_message(pong, strlen(pong), client->client_socket, connection);
    }
    else if (strcmp(token, "LOGIN") == 0)
    {
        token = strtok(NULL, "\n");
        asprintf(&(client->username), "%s", token);
        char *response = "10\n1\nLOGIN\n\nLogged in\n";
        send_message(response, strlen(response), client->client_socket,
                     connection);
    }
    else if (strcmp(token, "LIST-USERS") == 0)
    {}
    else if (strcmp(token, "SEND-DM") == 0)
    {
        // handle_param;
    }
    else if (strcmp(token, "BROADCAST") == 0)
    {
        // handle_param;
    }

    return connection;
}

// Reads incoming message from client and processes it.
struct connection_t *get_message(struct connection_t *connection, int connfd)
{
    char received[DEFAULT_BUFFER_SIZE];
    struct connection_t *client = find_client(connection, connfd);

    ssize_t n = recv(connfd, received, DEFAULT_BUFFER_SIZE, 0);

    if (n <= 0)
    {
        // Handle client disconnection or error.
        if (n == -1)
            perror("failed to read data from client");
        else
            puts("Client disconnected");

        connection = remove_client(connection, connfd);
    }
    else
    {
        client->buffer = xrealloc(client->buffer, client->nb_read + n);
        memcpy(client->buffer + client->nb_read, received, n);
        client->nb_read += n;

        connection = process_message(client, connection);
    }

    return connection;
}

int main(int argc, char **argv)
{
    if (argc != 3)
        errx(1, "usage: ./opichat_server <ip> <port>");

    // Get a listener socket for server.
    int listen_sock = prepare_socket(argv[1], argv[2]);

    // Create an epoll instance (file descriptor) and add listening socket to
    // set.
    struct epoll_event event;

    int epoll_instance = epoll_create1(0);

    if (epoll_instance == -1)
        err(1, "failed to create epoll instance");

    // Associate listening socket for read operations.
    event.events = EPOLLIN;
    event.data.fd = listen_sock;

    // Add listener socket fd to new epoll instance.
    // Event structure passed indicates that only input events will be handled.
    if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, listen_sock, &event) == -1)
        err(1, "failed to add listener socket to epoll instance");

    // Wait indefinitely for an event to occur.
    struct connection_t *connection = NULL;

    while (1)
    {
        // Create list for events that will occur from epoll instance
        // (ready list).
        struct epoll_event events[MAX_EVENTS];

        // Wait for events on given file descriptors.
        // events_count represents number of file descriptors on which
        // an event occured (ready file descriptors).
        int events_count = epoll_wait(epoll_instance, events, MAX_EVENTS, -1);

        if (events_count == -1)
            err(1, "failed to wait for events to occur");

        // Handle ready file descriptors.
        for (int event_idx = 0; event_idx < events_count; event_idx++)
        {
            // Listener socket.
            if (events[event_idx].data.fd == listen_sock)
            {
                // Accept a new client and add it to the connection_t struct.
                connection =
                    accept_client(epoll_instance, listen_sock, connection);
            }
            else
            {
                connection = get_message(connection, events[event_idx].data.fd);
            }
        }
    }
}
