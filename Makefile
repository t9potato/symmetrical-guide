CC=gcc
CFLAGS=
LFLAGS=
SRC=src
BIN=bin
MAIN=main

all:
	$(CC) $(CFLAGS) $(LFLAGS) $(SRC)/$(MAIN).c -o $(BIN)/$(MAIN)
