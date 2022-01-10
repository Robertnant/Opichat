CC= gcc
CFLAGS= -Werror -Wall -Wextra -pedantic -std=c99 -fsanitize=address \
	-D_POSIX_C_SOURCE=200809L
CPPFLAGS = -Isrc
LDFLAGS = -lcriterion

SRC = ./src/opichat_server.c ./src/connection.c ./src/utils/xalloc.c
BIN= opichat_server

TSRC = ./tests/tests.c
#OBJ = $(SRC:.c=.o)

all: opichat_server

opichat_server: $(SRC)
	$(CC) $(CFLAGS) $^ -o $(BIN)

debug: CFLAGS += -g
debug: all

check: $(SRC) $(TSRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(SRC) $(TSRC) -o test #$(LDFLAGS)
	./test

.PHONY: clean

clean:
	${RM} *.o test
