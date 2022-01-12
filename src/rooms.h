// List of rooms ordered in first in first out order.
struct room
{
    char *name;
    int owner;
    struct room *next;
};

struct rooms
{
    struct room *head;
    struct room *tail;
    size_t size;
};

// Returns a new initialized rooms list.
struct rooms *init_room(void)
{
    struct rooms *list = malloc(sizeof(struct rooms));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return list;
}

// Adds room to list and associates it with client connection.
struct rooms *create_room(char *name, struct *rooms,
                          struct connection_t *client);

// Deletes room from list and removes room association from client connection.
struct rooms *delete_room(char *name, int client_fd, struct *rooms,
                          struct connection_t *connections);

// Creates list of created rooms and sends to client.
void list_rooms(struct *rooms rooms, int client_fd);

struct rooms *join_room(char *name, int client_fd, struct *rooms rooms,
                        struct connection_t *client);

// Joins specified room if existing.
struct rooms *join_room(char *name, int client_fd, struct *rooms rooms,
                        struct connection_t *client);
