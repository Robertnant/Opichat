#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
** \brief Tokenizer for message received from client or server.
**
** \param message Pointer to the message buffer.
** Update to next message by tokenizer.
** \param tokens_count Pointer to the tokens that will be set by lexer.
** Updated by tokenizer.
** \return An allocated array containing the message tokens.
*/
char **lexer(char **message, int *tokens_count);

#endif /* !LEXER_H */
