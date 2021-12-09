CC= gcc
CFLAGS= -Werror -Wall -Wextra -pedantic -std=c99 -fsanitize=address \
	-D_POSIX_C_SOURCE=200809L

SRC= basic_client.c
BIN= basic_client

.PHONY= clean all

all: basic_client

basic_client: $(SRC)
	$(CC) $(CFLAGS) $^ -o $(BIN)

debug: CFLAGS += -g
debug: all

clean:
	$(RM) $(BIN)
