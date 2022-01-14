#define _GNU_SOURCE

#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "xalloc.h"

// Returns a new initialized queue list.
struct queue *init_queue(void)
{
    struct queue *list = xmalloc(sizeof(struct queue));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return list;
}

// Adds room to list and associates it with client connection.
char *add_room(char *name, struct queue *rooms, struct connection_t *client)
{
    // Create new element.
    struct list *element = xmalloc(sizeof(struct list));
    element->owner = client->client_socket;
    asprintf(&name, "%s", name);
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

    // return gen_message(...);
    return NULL;
}

// Deletes room from list and removes room association from clients.
char *delete_room(char *name, int client_fd, struct queue *rooms,
                  struct connection_t *connection)
{
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
    }
    else
    {
        // Check if owner matches current client.
        if (client_fd != curr->owner)
        {
            // return gen_message(...);
            return NULL;
        }

        if (curr == rooms->head)
            rooms->head = curr->next;

        if (curr == rooms->tail)
            rooms->tail = prev;

        if (prev)
            prev->next = curr->next;

        rooms->size -= 1;

        free(curr);
    }

    // Remove room from all client connections.
    while (connection != NULL)
    {
        if (strcmp(connection->room, name) == 0)
        {
            free(connection->room);
            connection->room = NULL;
        }
        connection = connection->next;
    }

    // return gen_message(...);
    return NULL;
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
        rooms_list = xrealloc(rooms_list, len + name_len);
        memcpy(rooms_list + len, curr->name, name_len);
        len += name_len;

        // Add newline.
        rooms_list[len - 1] = '\n';
    }

    // Create payload data structure with NULL parameters.
    struct params_payload *data = xcalloc(1, sizeof(struct params_payload));
    data->payload = rooms_list;

    char *message = gen_message(len, 1, "LIST_ROOMS", data);

    return message;
}

// Joins specified room if existing.
char *join_room(char *name, struct queue *rooms, struct connection_t *client)
{
    // Check if room exists.
    struct list *curr = rooms->head;
    while (curr != NULL && strcmp(curr->name, name) != 0)
    {
        curr = curr->next;
    }

    if (curr == NULL)
    {
        // Error handling.
    }

    if (client->room)
        free(client->room);

    asprintf(&(client->room), "%s", name);

    // return gen_message(...);
    return NULL;
}
