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

#include "utils/lexer.h"
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

// Returns a new initialized queue list.
struct queue *init_queue(void)
{
    struct queue *list = xmalloc(sizeof(struct queue));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return list;
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
            // Get IP address of client.
            char ip[DEFAULT_BUFFER_SIZE]; // to a
            getnameinfo(&client, connfd_len, ip, DEFAULT_BUFFER_SIZE, NULL, 0,
                        NI_NUMERICHOST);

            connection = add_client(connection, connfd, ip);
            connection->rooms = init_queue();
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

// Finds user or room with specified name. One structure parameter should be
// NULL while the other not. If both NULL returns NULL.
void *find_element(struct connection_t *connection, struct queue *rooms,
                   char *name)
{
    if (rooms != NULL)
    {
        struct list *curr = rooms->head;

        while (curr)
        {
            if (strcmp(curr->name, name) == 0)
                return curr;

            curr = curr->next;
        }
    }
    else if (connection != NULL)
    {
        struct connection_t *curr = connection;

        while (curr)
        {
            if (curr->username && strcmp(curr->username, name) == 0)
                return curr;

            curr = curr->next;
        }
    }

    return NULL;
}

struct connection_t *process_message(struct connection_t *client,
                                     struct connection_t *connection,
                                     struct queue *rooms)
{
    // Tokenize and parse each message.
    int count = 0;
    char *next_message = client->buffer;

    while (next_message[0])
    {
        struct params_payload *p = xcalloc(1, sizeof(struct params_payload));

        char **tokens = lexer(&next_message, &count);
        char *response = NULL;

        if (tokens == NULL)
            return connection;

        // Check for invalid request.
        if (strcmp(tokens[1], "0") != 0)
        {
            // Send an invalid request message.
            // Might need to use a pointer to error code as parameter to set
            // error code for message parser.
            puts("invalid request");
        }

        char *command = tokens[2];

        if (strcmp(command, "PING") == 0)
        {
            asprintf(&response, "5\n1\nPING\n\nPONG\n");
        }
        else if (strcmp(command, "LOGIN") == 0)
        {
            if (strcmp(tokens[count - 1], "") == 0
                || !is_valid(tokens[count - 1]))
            {
                asprintf(&response, "13\n3\nLOGIN\n\nBad username\n");
            }
            else
            {
                struct connection_t *el =
                    find_element(connection, NULL, tokens[count - 1]);

                if (el)
                {
                    asprintf(&response, "19\n3\nLOGIN\n\nDuplicate username\n");
                }
                else
                {
                    asprintf(&(client->username), "%s", tokens[count - 1]);
                    asprintf(&response, "10\n1\nLOGIN\n\nLogged in\n");
                }
            }
        }
        else if (strcmp(command, "LIST-USERS") == 0)
        {
            // Create a payload with usernames and generate message.
            char *users = NULL;
            size_t len = 0;

            struct connection_t *curr = connection;
            while (curr)
            {
                if (curr->username)
                {
                    // Prepend username to result.
                    size_t user_len = strlen(curr->username);

                    users = xrealloc(users, len + user_len + 2);
                    memmove(users + user_len + 1, users, len);
                    memcpy(users, curr->username, user_len);
                    users[user_len] = '\n';

                    len += user_len + 1;
                    users[len] = '\0';
                }

                curr = curr->next;
            }

            p->payload = users;

            response = gen_message(len, 1, "LIST-USERS", p);
        }
        else if (strcmp(command, "SEND-DM") == 0)
        {
            // Get username parameter.
            // TODO Maybe set value for i.
            int i;
            for (i = 2; i < count; i++)
            {
                if (strstr(tokens[i], "User=") != NULL)
                    break;
            }

            if (i == count)
            {
                asprintf(&response, "12\n3\nSEND-DM\n\nBad request\n");
            }
            else
            {
                // Check if user exists.
                struct connection_t *curr = connection;
                while (curr)
                {
                    int offset = strlen("User=");
                    if (curr->username
                        && strcmp(tokens[i] + offset, curr->username) == 0)
                        break;

                    curr = curr->next;
                }

                if (curr == NULL)
                {
                    asprintf(&response, "15\n3\nSEND-DM\n%s\nUser not found\n",
                             tokens[i]);
                }
                else
                {
                    // Send response and notification.
                    p->params = add_param(p->params, tokens[i], NULL);

                    if (client->username)
                    {
                        p->params =
                            add_param(p->params, "From", client->username);
                    }
                    else
                    {
                        p->params = add_param(p->params, "From", "<Anonymous>");
                    }

                    asprintf(&p->payload, "%s", tokens[count - 1]);
                    size_t len = atol(tokens[0]);

                    response = gen_message(len, 2, command, p);
                    send_message(response, strlen(response),
                                 curr->client_socket, connection);

                    free(response);

                    asprintf(&response, "0\n1\nSEND-DM\n%s\n\n", tokens[i]);
                }
            }
        }
        else if (strcmp(command, "BROADCAST") == 0)
        {
            // handle response and notification
        }
        else if (strcmp(command, "CREATE-ROOM") == 0)
        {
            if (strcmp(tokens[count - 1], "") == 0
                || !is_valid(tokens[count - 1]))
            {
                asprintf(&response, "14\n3\nCREATE-ROOM\n\nBad room name\n");
            }
            else
            {
                struct list *el = find_element(NULL, rooms, tokens[count - 1]);
                if (el)
                {
                    asprintf(&response,
                             "20\n3\nCREATE-ROOM\n\nDuplicate room name\n");
                }
                else
                {
                    response = add_room(tokens[count - 1], rooms, client);
                }
            }
        }
        else if (strcmp(command, "LIST-ROOMS") == 0)
        {
            char *rooms_list = list_rooms(rooms);
            size_t len = rooms_list ? strlen(rooms_list) : 0;
            p->payload = rooms_list;
            response = gen_message(len, 1, "LIST-ROOMS", p);
        }
        else if (strcmp(command, "JOIN-ROOM") == 0)
        {
            response = join_room(tokens[count - 1], rooms, client);
        }
        else if (strcmp(command, "LEAVE-ROOM") == 0)
        {
            response = leave_room(tokens[count - 1], rooms, client);
        }
        else if (strcmp(command, "SEND-ROOM") == 0)
        {}
        else if (strcmp(command, "DELETE-ROOM") == 0)
        {
            response = delete_room(tokens[count - 1], client->client_socket,
                                   rooms, connection);
        }
        else if (strcmp(command, "PROFILE") == 0)
        {
            // Create payload.
            char *data = list_rooms(client->rooms);
            size_t len = 0;

            if (data)
            {
                if (client->username)
                {
                    len = asprintf(&p->payload,
                                   "Username: %s\nIP: %s\nRooms:\n%s",
                                   client->username, client->ip, data);
                }
                else
                {
                    len = asprintf(&p->payload,
                                   "Username: <Anonymous>\nIP: %s\nRooms:\n%s",
                                   client->ip, data);
                }
            }
            else
            {
                if (client->username)
                {
                    len =
                        asprintf(&p->payload, "Username: %s\nIP: %s\nRooms:\n",
                                 client->username, client->ip);
                }
                else
                {
                    len = asprintf(&p->payload,
                                   "Username: <Anonymous>\nIP: %s\nRooms:\n",
                                   client->ip);
                }
            }

            response = gen_message(len, 1, "PROFILE", p);
        }

        // Send and free response.
        if (response)
        {
            send_message(response, strlen(response), client->client_socket,
                         connection);
            free(response);
        }

        free_payload(p);
        p = NULL;

        // TODO Free tokens array.
    }

    // Reset client buffer if parsing successful (tokens created at least
    // once).
    if (count != 0)
    {
        client->nb_read = 0;
        free(client->buffer);
        client->buffer = NULL;
    }

    return connection;
}

// Reads incoming message from client and processes it.
struct connection_t *get_message(struct connection_t *connection, int connfd,
                                 struct queue *rooms)
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

        // Delete rooms associated to client and remove client.
        struct list *curr = client->rooms->head;
        struct list *next = NULL;
        while (curr)
        {
            char *res = NULL;
            next = curr->next;

            if (curr->owner == client->client_socket)
            {
                res = delete_room(curr->name, client->client_socket, rooms,
                                  connection);
            }
            else
            {
                res = leave_room(curr->name, client->rooms, client);
            }

            free(res);
            curr = next;
        }
        connection = remove_client(connection, connfd);
    }
    else
    {
        // Realloc buffer and NULL terminate for parsing.
        client->buffer = xrealloc(client->buffer, client->nb_read + n + 1);
        memcpy(client->buffer + client->nb_read, received, n);
        client->nb_read += n;
        client->buffer[client->nb_read] = '\0';

        connection = process_message(client, connection, rooms);
    }

    return connection;
}

int main(int argc, char **argv)
{
    if (argc != 3)
        errx(1, "usage: %s <ip> <port>", argv[0]);

    // Get a listener socket for server.
    int listen_sock = prepare_socket(argv[1], argv[2]);

    // Create an epoll instance (file descriptor) and add listening socket
    // to set.
    struct epoll_event event;

    int epoll_instance = epoll_create1(0);

    if (epoll_instance == -1)
        err(1, "failed to create epoll instance");

    // Associate listening socket for read operations.
    event.events = EPOLLIN;
    event.data.fd = listen_sock;

    // Add listener socket fd to new epoll instance.
    // Event structure passed indicates that only input events will be
    // handled.
    if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, listen_sock, &event) == -1)
        err(1, "failed to add listener socket to epoll instance");

    // Initialize rooms structure.
    struct queue *rooms = init_queue();

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
                // Accept a new client and add it to the connection_t
                // struct.
                connection =
                    accept_client(epoll_instance, listen_sock, connection);
            }
            else
            {
                connection =
                    get_message(connection, events[event_idx].data.fd, rooms);
            }
        }
    }
}
