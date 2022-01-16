#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"

// Structure to store payload and parameters.
struct params_payload
{
    char *payload;
    struct list *params;
};

/**
** \brief Tokenizer for message received from client or server.
**
** \param message Pointer to the message buffer. Should not be the pointer
** used in recv when listening for message.
** Update to next message by tokenizer.
** \param tokens_count Pointer to the tokens that will be set by lexer.
** Updated by tokenizer.
** \return An allocated array containing the message tokens or NULL if message
** is partial.
*/
char **lexer(char **message, int *tokens_count);

/*
** Adds parameter to parameters list (order does not matter).
** If value is NULL, then key contains both key and value pair.
*/
struct list *add_param(struct list *params, char *key, char *value);

// Checks if parameter or payload is valid. Returns 1 if valid else 0.
int is_valid(char *element);

// Generates messages ready to be sent to server or client.
char *gen_message(size_t size, int status, char *command,
                  struct params_payload *p);

// Creates response messages by first adding previous request parameters.
char *generate_response(char **tokens, int count, struct params_payload *p,
                        int status);

// Fres params_payload structure.
void free_payload(struct params_payload *p);

#endif /* !LEXER_H */
