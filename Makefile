CC=gcc
CFLAGS=-g
LFLAGS=-lSDL2 -lm
SRC=src
BIN=build
MAIN=main

all:
	$(CC) $(CFLAGS) $(LFLAGS) $(SRC)/$(MAIN).c -o $(BIN)/$(MAIN)

run: all
	./$(BIN)/$(MAIN)

r: run
