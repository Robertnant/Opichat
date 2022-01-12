// List of rooms ordered in first in first out order.
struct list
{
    char *name;
    int owner;
    struct list *next;
};

struct params_payload
{
    char *payload;
    struct list *params;
};

struct queue
{
    struct list *head;
    struct list *tail;
    size_t size;
};

// Returns a new initialized queue list.
struct queue *init_queue(void)
{
    struct queue *list = malloc(sizeof(struct queue));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return list;
}

// Adds room to list and associates it with client connection.
struct queue *create_room(char *name, struct *queue,
                          struct connection_t *client);

// Deletes room from list and removes room association from client connection.
struct queue *delete_room(char *name, int client_fd, struct *queue,
                          struct connection_t *connections);

// Creates list of created rooms and sends to client.
void list_queue(struct *queue queue, int client_fd);

// Joins specified room if existing.
struct queue *join_room(char *name, int client_fd, struct *queue queue,
                        struct connection_t *client);

// Generate message
char *gen_message(size_t size, int status, char *command, 
        struct params_payload *p);
