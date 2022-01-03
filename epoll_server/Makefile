CC= gcc
CFLAGS= -Werror -Wall -Wextra -pedantic -std=c99 -fsanitize=address \
	-D_POSIX_C_SOURCE=200809L

SRC= epoll_server.c
BIN= epoll_server

.PHONY= clean all

all: epoll_server

basic_client: $(SRC)
	$(CC) $(CFLAGS) $^ -o $(BIN)

debug: CFLAGS += -g
debug: all

clean:
	$(RM) $(BIN)
