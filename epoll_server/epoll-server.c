#include "epoll-server.h"

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/epoll.h>

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
    // TODO: Rajouter code de Clarel.
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
    // TODO: Rajouter code de Clarel.
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
    // connection_t struct is only useful when needing to know from where
    // to resume reading data from a specific client.
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: ./epoll_server SERVER_IP SERVER_PORT\n");
        return 1;
    }

    // Get a listener socket.
    int listener_fd = prepare_socket(argv[1], argv[2]);

    // Create an epoll instance (file descriptor) and add listening socket to set.
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
    while (true)
    {
        // Create list for events that will occur from epoll instance
        // (ready list).
        struct epoll_events events[MAX_EVENTS];

        // Wait for events on given file descriptors.
        // events_count represents number of file descriptors on which
        // an event occured (ready file descriptors).
        int events_count = epoll_wait(epoll_instance, events, MAX_EVENTS, -1);

        if (events_count == -1)
            err(1, "failed to wait for events to occur on given socket");

        // Handle ready file descriptors.
        for (int event_idx = 0; event_idx < events_count; event_idx++)
        {
            // Listener socket.
            if (events[event_idx].data.fd == listen_sock)
            {

            }
        }
    }
}
