# Simple makefile for utils
CC=gcc
WCC=i686-w64-mingw32-gcc
SRC=src
WBIN=win32
BIN=bin
INSTALL_DIR=~/.local/bin

all: bin2btx txt2bas btx2bin

bin2btx: $(SRC)/bin2btx.c
	$(CC) -o $(BIN)/bin2btx $(SRC)/bin2btx.c -lm
	$(WCC) -o $(WBIN)/bin2btx $(SRC)/bin2btx.c -lm

txt2bas: $(SRC)/txt2bas.c
	$(CC) -o $(BIN)/txt2bas $(SRC)/txt2bas.c -lm
	$(WCC) -o $(WBIN)/txt2bas $(SRC)/txt2bas.c -lm

btx2bin: $(SRC)/btx2bin.c
	$(CC) -o $(BIN)/btx2bin $(SRC)/btx2bin.c -lm
	$(WCC) -o $(WBIN)/btx2bin $(SRC)/btx2bin.c -lm

clean:
	rm -f *~ $(SRC)/*~ 

install:
	cp $(BIN)/* $(INSTALL_DIR)/
