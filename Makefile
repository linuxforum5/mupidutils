# Simple makefile for utils
CC=gcc
WCC=i686-w64-mingw32-gcc
SRC=src
WBIN=win32
BIN=bin
INSTALL_DIR=~/.local/bin

all: bin2btx

bin2btx: $(SRC)/bin2btx.c
	$(CC) -o $(BIN)/bin2btx $(SRC)/bin2btx.c -lm
	$(WCC) -o $(WBIN)/bin2btx $(SRC)/bin2btx.c -lm

clean:
	rm -f *~ $(SRC)/*~ 

install:
	cp $(BIN)/* $(INSTALL_DIR)/
