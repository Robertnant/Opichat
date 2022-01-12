
// Adds room to list and associates it with client connection.
struct rooms *create_room(char *name, struct *rooms,
                          struct connection_t *client)
{
    // Create new element.
    struct room *element = xmalloc(sizeof(struct room));
    element->owner = client->client_socket;
    asprintf(&name, "%s", name);
    element->next = NULL;

    // Handle case for empty rooms list.
    if (!(fifo->tail))
    {
        fifo->head = element;
        fifo->tail = element;
    }
    else
    {
        fifo->tail->next = element;
        fifo->tail = element;
    }

    fifo->size += 1;

    // Associate room to current client.
    if (client->room)
        free(client->room);

    asprintf(&(client->room), "%s", name);
}

// Deletes room from list and removes room association from client connection.
struct rooms *delete_room(char *name, int client_fd, struct *rooms rooms,
                          struct connection_t *connection)
{
    // Find room with given name.
    int index = 0;

    struct room *curr = rooms->head;

    while (curr != NULL && strcmp(curr->name, name) != 0)
    {
        index++;
        curr = curr->next
    }

    if (curr == NULL)
    {
        // Error handling.
    }
    else
    {
        // Check if index points to head.
        if (curr == list->head)
            list->head = curr->next;

        // Check if index points to tail.
        if (curr == list->tail)
            list->tail = curr->prev;

        // Unlink current node to delete from previous and next nodes.
        if (curr->prev)
            curr->prev->next = curr->next;

        if (curr->next)
            curr->next->prev = curr->prev;

        // Retrieve data and decrease size.
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

    return rooms;
}

// Creates list of created rooms and sends to client.
void list_rooms(struct *rooms rooms, int client_fd)
{
    char *rooms_list = NULL;
    size_t len = 0;

    struct *room curr = rooms->head;
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
    struct payload_data *data = xcalloc(sizeof(struct payload_data));
    data->payload = rooms_list;

    char *message = gen_message(len, 1, "LIST_ROOMS", data);
    ssize_t err = resend(client_fd, message, strlen(message), 0);
}

// Joins specified room if existing.
struct rooms *join_room(char *name, int client_fd, struct *rooms rooms,
                        struct connection_t *client)
{
    // Check if room exists.
    struct *room curr = rooms->head;
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
}
