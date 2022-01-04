CC= gcc
CFLAGS= -Werror -Wall -Wextra -pedantic -std=c99 -fsanitize=address
	-D_POSIX_C_SOURCE=200809L

SRC= basic_server.c
BIN= basic_server

.PHONY= clean all

all: basic_server

basic_server: $(SRC)
	$(CC) $(CFLAGS) $^ -o $(BIN)

debug: CFLAGS += -g
debug: all

clean:
	$(RM) $(BIN)
