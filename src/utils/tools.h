#ifndef TOOLS_H
#define TOOLS_H

#include "../connection.h"

// Queue element.
struct list
{
    char *name;
    int owner;
    struct list *next;
};

// Queue.
struct queue
{
    struct list *head;
    struct list *tail;
    size_t size;
};

// Adds room to list and associates it with client connection.
char *add_room(char *name, struct queue *rooms, struct connection_t *client);

// Deletes room from list and removes room association from clients.
char *leave_room(char *name, struct queue *rooms, struct connection_t *client);

// Deletes room from list and removes room association from client connection.
char *delete_room(char *name, int client_fd, struct queue *rooms,
                  struct connection_t *connection);

// Creates list of created rooms and sends to client.
char *list_rooms(struct queue *rooms);

// Joins specified room if existing.
char *join_room(char *name, struct queue *rooms, struct connection_t *client);

#endif /* TOOLS_H */
