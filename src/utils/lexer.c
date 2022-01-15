#define _GNU_SOURCE

#include "lexer.h"

#include "xalloc.h"

char **lexer(char **message, int *tokens_count)
{
    char *receive = *message;

    *tokens_count = 3;
    char **tokens = xcalloc(*tokens_count, sizeof(char *));

    char *token = NULL;
    char *save = NULL;

    token = strtok_r(receive, "\n", &save);
    asprintf(&tokens[0], "%s", token);

    // Get payload to prevent making next message unusable.
    size_t payload_size = atoll(tokens[0]);
    char *payload = strstr(save, "\n\n");
    payload = payload ? payload + 2 : payload;

    // Check if message received is partial.
    if (payload == NULL || strlen(payload) < payload_size)
    {
        free(tokens[0]);
        free(tokens);

        return NULL;
    }

    // Get pointer to next message command received.
    char *next_message = payload + payload_size;

    if (strcmp(tokens[0], "0") != 0)
    {
        char *payload_cpy = payload;
        asprintf(&payload, "%s", payload);

        // Null terminate begining of payload in received data.
        *(payload_cpy - 2) = '\0';
    }

    // Save status and command.
    for (int i = 1; i < 3; i++)
    {
        token = strtok_r(NULL, "\n", &save);
        asprintf(&tokens[i], "%s", token);
    }

    // Get data parameters.
    token = strtok_r(NULL, "\n", &save);

    while (token)
    {
        *tokens_count += 1;
        tokens = xrealloc(tokens, *tokens_count * sizeof(char *));
        asprintf(&tokens[*tokens_count - 1], "%s", token);

        token = strtok_r(NULL, "\n", &save);
    }

    // Add delimiter after parameters.
    *tokens_count += 1;
    tokens = xrealloc(tokens, *tokens_count * sizeof(char *));
    asprintf(&tokens[*tokens_count - 1], "%s", "");

    if (payload[0])
    {
        *tokens_count += 1;

        tokens = xrealloc(tokens, *(tokens_count) * sizeof(char *));
        tokens[*tokens_count - 1] = xcalloc(payload_size + 1, sizeof(char));
        memcpy(tokens[*tokens_count - 1], payload, payload_size);
        free(payload);
    }

    // Update message pointer to next message.
    *message = next_message;

    return tokens;
}

// Checks if parameter or payload is valid. Returns 1 if valid else 0.
int is_valid(char *element)
{
    size_t i = 0;
    while (element[i])
    {
        int cond1 = element[i] >= 'a' && element[i] <= 'z';
        int cond2 = element[i] >= 'A' && element[i] <= 'Z';
        int cond3 = element[i] >= '0' && element[i] <= '9';

        if (!cond1 && !cond2 && !cond3)
            return 0;

        i++;
    }

    return 1;
}
// Adds parameter to parameters list (order does not matter).
struct list *add_param(struct list *params, char *key, char *value)
{
    struct list *new = xcalloc(1, sizeof(struct list));
    asprintf(&(new->name), "%s=%s", key, value);

    new->next = params;

    return new;
}

char *gen_message(size_t size, int status, char *command,
                  struct params_payload *p)
{
    char *res = NULL;
    size_t count = asprintf(&res, "%ld\n%d\n%s\n", size, status, command);

    while (p->params != NULL)
    {
        res =
            xrealloc(res, (count + strlen(p->params->name) + 1) * sizeof(char));
        count += sprintf(res + count, "%s\n", p->params->name);
        p->params = p->params->next;
    }

    res = xrealloc(res, count + 2);
    count += sprintf(res + count, "\n");

    if (size)
    {
        res = xrealloc(res, (count + size + 1) * sizeof(char));
        count += sprintf(res + count, "%s", p->payload);
    }
    return res;
}

// Fres params_payload structure.
void free_payload(struct params_payload *p)
{
    if (!p)
        return;

    if (p->payload)
        free(p->payload);

    struct list *curr = p->params;
    struct list *next = NULL;

    while (curr)
    {
        next = curr->next;

        free(curr->name);
        free(curr);

        curr = next;
    }

    free(p);
}
