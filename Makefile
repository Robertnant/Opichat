CC= gcc
CFLAGS= -Werror -Wall -Wextra -pedantic -std=c99 -fsanitize=address \
	-D_POSIX_C_SOURCE=200809L
CPPFLAGS = -Isrc
LDFLAGS = -lcriterion

SERVER_SRC = ./src/opichat_server.c ./src/connection.c ./src/utils/xalloc.c
CLIENT_SRC = ./src/opichat_client.c ./src/connection.c ./src/utils/xalloc.c
SERVER_BIN= opichat_server
CLIENT_BIN= opichat_client

TSRC = ./tests/tests.c

all: opichat_server opichat_client

opichat_server: $(SERVER_SRC)
	$(CC) $(CFLAGS) $^ -o $(SERVER_BIN)

opichat_client: $(CLIENT_SRC)
	$(CC) $(CFLAGS) $^ -o $(CLIENT_BIN)

debug: CFLAGS += -g
debug: all

check: $(SRC) $(TSRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(CLIENT_SRC) \
	    $(SERVER_SRC) $(TSRC) -o test
	./test

.PHONY: clean

clean:
	${RM} *.o $(CLIENT_BIN) $(SERVER_BIN) test
