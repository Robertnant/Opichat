#define _GNU_SOURCE

#include "connection.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils/xalloc.h"

struct connection_t *add_client(struct connection_t *connection,
                                int client_socket, char *ip)
{
    struct connection_t *new_connection = xmalloc(sizeof(struct connection_t));

    new_connection->client_socket = client_socket;
    new_connection->username = NULL;
    new_connection->rooms = NULL;
    new_connection->buffer = NULL;
    asprintf(&new_connection->ip, "%s", ip);
    new_connection->nb_read = 0;
    new_connection->next = connection;

    return new_connection;
}

struct connection_t *remove_client(struct connection_t *connection,
                                   int client_socket)
{
    if (connection && connection->client_socket == client_socket)
    {
        struct connection_t *client_connection = connection->next;
        if (close(connection->client_socket) == -1)
            errx(1, "Failed to close socket");
        free(connection->username);

        if (connection->rooms)
            free(connection->rooms);

        free(connection->ip);
        free(connection->buffer);
        free(connection);
        return client_connection;
    }

    struct connection_t *tmp = connection;
    while (tmp->next)
    {
        if (tmp->next->client_socket == client_socket)
        {
            struct connection_t *client_connection = tmp->next;
            tmp->next = client_connection->next;
            if (close(client_connection->client_socket) == -1)
                errx(1, "Failed to close socket");
            free(client_connection->username);

            if (client_connection->rooms)
                free(client_connection->rooms);

            free(connection->ip);
            free(client_connection->buffer);
            free(client_connection);
            break;
        }
        tmp = tmp->next;
    }

    return connection;
}

struct connection_t *find_client(struct connection_t *connection,
                                 int client_socket)
{
    while (connection != NULL && connection->client_socket != client_socket)
        connection = connection->next;

    return connection;
}
