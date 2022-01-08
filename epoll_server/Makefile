CC= gcc
CFLAGS= -Werror -Wall -Wextra -pedantic -std=c99 -fsanitize=address \
	-D_POSIX_C_SOURCE=200809L

SRC= epoll-server.c connection.c ./utils/xalloc.c
BIN= epoll-server

.PHONY= clean all

all: epoll_server

epoll_server: $(SRC)
	$(CC) $(CFLAGS) $^ -o $(BIN)

debug: CFLAGS += -g
debug: all

clean:
	$(RM) $(BIN)
