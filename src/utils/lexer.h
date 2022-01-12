#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
** \brief Tokenizer for message received from client or server.
**
** \param receive The message buffer.
** \param tokens_count Pointer to the tokens that will be set by lexer.
** \param next_message Pointer to the next message received. Pointer is
** updated by tokenizer.
** \return An allocated array containing the message tokens.
*/
char **lexer(char *receive, int *tokens_count, char **next_message);

#endif /* !LEXER_H */
