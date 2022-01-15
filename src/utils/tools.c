#define _GNU_SOURCE

#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "xalloc.h"

// Adds room to list and associates it with client connection.
char *add_room(char *name, struct queue *rooms, struct connection_t *client)
{
    // Create new element.
    struct list *element = xmalloc(sizeof(struct list));
    element->owner = client->client_socket;
    asprintf(&element->name, "%s", name);
    element->next = NULL;

    // Handle case for empty rooms list.
    if (!(rooms->tail))
    {
        rooms->head = element;
        rooms->tail = element;
    }
    else
    {
        rooms->tail->next = element;
        rooms->tail = element;
    }

    rooms->size += 1;

    // Associate room to current client.
    if (client->room)
        free(client->room);

    asprintf(&(client->room), "%s", name);

    char *response = NULL;
    asprintf(&response, "13\n1\nCREATE-ROOM\n\nRoom created\n");

    return response;
}

// Removes room association from client.
char *leave_room(char *name, struct queue *rooms, struct connection_t *client)
{
    char *response = NULL;

    // Find room with given name.
    struct list *curr = rooms->head;

    while (curr != NULL && strcmp(curr->name, name) != 0)
    {
        curr = curr->next;
    }

    if (curr == NULL)
    {
        // Error handling.
        asprintf(&response, "15\n3\nLEAVE-ROOM\n\nRoom not found\n");
    }
    else
    {
        free(client->room);
        client->room = NULL;
        asprintf(&response, "10\n1\nLEAVE-ROOM\n\nRoom left\n");
    }

    return response;
}

// Deletes room from list and removes room association from clients.
char *delete_room(char *name, int client_fd, struct queue *rooms,
                  struct connection_t *connection)
{
    char *response = NULL;

    // Find room with given name.
    struct list *curr = rooms->head;
    struct list *prev = NULL;

    while (curr != NULL && strcmp(curr->name, name) != 0)
    {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL)
    {
        // Error handling.
        asprintf(&response, "15\n1\nDELETE-ROOM\n\nRoom not found\n");
        return response;
    }
    else
    {
        // Check if owner matches current client.
        if (client_fd != curr->owner)
        {
            asprintf(&response, "13\n1\nDELETE-ROOM\n\nUnauthorized\n");
            return response;
        }

        if (curr == rooms->head)
            rooms->head = curr->next;

        if (curr == rooms->tail)
            rooms->tail = prev;

        if (prev)
            prev->next = curr->next;

        rooms->size -= 1;

        free(curr->name);
        curr->name = NULL;
        free(curr);
    }

    // Remove room from all client connections.
    while (connection != NULL)
    {
        if (connection->room && strcmp(connection->room, name) == 0)
        {
            free(connection->room);
            connection->room = NULL;
        }
        connection = connection->next;
    }

    asprintf(&response, "13\n1\nDELETE-ROOM\n\nRoom deleted\n");

    return response;
}

// Creates list of created rooms and generates response message.
char *list_rooms(struct queue *rooms)
{
    char *rooms_list = NULL;
    size_t len = 0;

    struct list *curr = rooms->head;
    while (curr != NULL)
    {
        // Gets name length including newline to separate names in list.
        size_t name_len = strlen(curr->name) + 1;
        rooms_list = xrealloc(rooms_list, len + name_len + 1);
        memcpy(rooms_list + len, curr->name, name_len);
        len += name_len;

        // Add newline.
        rooms_list[len - 1] = '\n';

        curr = curr->next;
    }

    // Add null termination.
    if (len)
        rooms_list[len] = '\0';

    // Create payload data structure with NULL parameters.
    struct params_payload *data = xcalloc(1, sizeof(struct params_payload));
    data->payload = rooms_list;

    char *message = gen_message(len, 1, "LIST-ROOMS", data);

    return message;
}

// Joins specified room if existing.
char *join_room(char *name, struct queue *rooms, struct connection_t *client)
{
    char *response = NULL;

    // Check if room exists.
    struct list *curr = rooms->head;
    while (curr != NULL && strcmp(curr->name, name) != 0)
    {
        curr = curr->next;
    }

    if (curr == NULL)
    {
        // Error handling.
        asprintf(&response, "15\n3\nJOIN-ROOM\n\nRoom not found\n");
        return response;
    }

    if (client->room)
        free(client->room);

    asprintf(&(client->room), "%s", name);

    asprintf(&response, "12\n1\nJOIN-ROOM\n\nRoom joined\n");

    return response;
}
