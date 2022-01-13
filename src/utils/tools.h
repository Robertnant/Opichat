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

// Returns a new initialized queue list.
struct queue *init_queue(void);

// Adds room to list and associates it with client connection.
struct queue *create_room(char *name, struct queue *rooms,
                          struct connection_t *client);

// Deletes room from list and removes room association from client connection.
struct queue *delete_room(char *name, int client_fd, struct queue *rooms,
                          struct connection_t *connection);

// Creates list of created rooms and sends to client.
char *list_rooms(struct queue *rooms);

// Joins specified room if existing.
void join_room(char *name, struct queue *rooms, struct connection_t *client);

#endif /* TOOLS_H */
