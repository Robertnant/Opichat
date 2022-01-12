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

    // Get pointer to next message command received.
    // char *next_message = strstr(save, "\n\n") + 2 + atoll(tokens[0]);

    // Get payload before strtok use.
    char *payload = NULL;

    if (strcmp(tokens[0], "0") != 0)
    {
        payload = strstr(save, "\n\n");
        char *payload_cpy = payload;
        payload += 2;
        asprintf(&payload, "%s", payload);

        // Null terminate begining of payload in received data.
        *payload_cpy = '\0';
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

    if (payload)
    {
        size_t payload_size = atoll(tokens[0]);
        *tokens_count += 1;

        tokens = xrealloc(tokens, *(tokens_count) * sizeof(char *));
        tokens[*tokens_count - 1] = xcalloc(payload_size + 1, sizeof(char));
        memcpy(tokens[*tokens_count - 1], payload, payload_size);
        // free(payload);
    }

    // Update message pointer to next message.
    // *message = next_message;

    return tokens;
}
