CC = gcc

SRC = $(wildcard src/*.c)
BIN = $(patsubst src/%.c, bin/%, $(SRC))

all: $(BIN)

bin/%: src/%.c
	$(CC) $< -o $@

clean:
	rm -f bin/*
