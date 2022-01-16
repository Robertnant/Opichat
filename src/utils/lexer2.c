#define _GNU_SOURCE

#include "lexer.h"
#include "xalloc.h"

// Generates response/notification/error by readding previous client params.
char *generate_response(char **tokens, int count, struct params_payload *p,
                        int status)
{
    int end = count - 2;
    if (strcmp(tokens[count - 1], "") == 0)
        end++;

    // With index, no parameter is added if command does not take any.
    for (int i = 3; i <= end; i++)
    {
        p->params = add_param(p->params, tokens[i], NULL);
    }

    if (p->payload == NULL)
        return gen_message(0, status, tokens[2], p);

    return gen_message(strlen(p->payload), status, tokens[2], p);
}
