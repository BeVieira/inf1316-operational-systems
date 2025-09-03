CC = gcc
CFLAGS = -Wall -Wextra

SRC = $(wildcard src/*.c)
BIN = $(patsubst src/%.c, bin/%, $(SRC))

all: $(BIN)

bin/%: src/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f bin/*
