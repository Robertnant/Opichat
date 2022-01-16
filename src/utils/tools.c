#define _GNU_SOURCE

#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "xalloc.h"

// Pushes a new element to a queue.
void push_element(char *name, int value, struct queue *queue)
{
    // Create new element.
    struct list *element = xmalloc(sizeof(struct list));
    element->owner = value;
    asprintf(&element->name, "%s", name);
    element->next = NULL;

    // Handle case for empty rooms list.
    if (!(queue->tail))
    {
        queue->head = element;
        queue->tail = element;
    }
    else
    {
        queue->tail->next = element;
        queue->tail = element;
    }

    queue->size += 1;
}

// Unlinks element from list.
void unlink_element(struct list *curr, struct list *prev, struct queue *queue)
{
    if (curr == queue->head)
        queue->head = curr->next;

    if (curr == queue->tail)
        queue->tail = prev;

    if (prev)
        prev->next = curr->next;

    queue->size -= 1;

    free(curr->name);
    curr->name = NULL;
    free(curr);
}

// Adds room to list and associates it with client connection.
char *add_room(char *name, struct queue *rooms, struct connection_t *client)
{
    push_element(name, client->client_socket, rooms);

    // Associate room to current client.
    push_element(name, client->client_socket, client->rooms);

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
        // Find first occurence of room.
        curr = client->rooms->head;
        struct list *prev = NULL;

        while (strcmp(curr->name, name) != 0)
        {
            prev = curr;
            curr = curr->next;
        }

        unlink_element(curr, prev, client->rooms);

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
        asprintf(&response, "15\n3\nDELETE-ROOM\n\nRoom not found\n");
        return response;
    }

    // Check if owner matches current client.
    if (client_fd != curr->owner)
    {
        asprintf(&response, "13\n3\nDELETE-ROOM\n\nUnauthorized\n");
        return response;
    }

    // Create copy of room name to prevent heap use after free.
    char *room_name = NULL;
    asprintf(&room_name, "%s", name);

    unlink_element(curr, prev, rooms);

    // Remove room from all client connections.
    while (connection != NULL)
    {
        curr = connection->rooms->head;
        prev = NULL;

        // Find first occurence of room.
        while (curr && strcmp(curr->name, room_name) != 0)
        {
            prev = curr;
            curr = curr->next;
        }

        if (curr)
        {
            unlink_element(curr, prev, connection->rooms);
            break;
        }

        connection = connection->next;
    }

    free(room_name);

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

    push_element(name, client->client_socket, client->rooms);

    asprintf(&response, "12\n1\nJOIN-ROOM\n\nRoom joined\n");

    return response;
}
