CC=gcc
SRC=src
INC=include
OBJ=obj
BIN=bin
CFLAGS=-Wall -I$(INC) -g

TARGETS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(wildcard $(SRC)/*.c))

.PHONY: all clean install uninstall

all: $(BIN)/graphX

$(BIN)/graphX: $(TARGETS) | $(BIN)
	$(CC) $(CFLAGS) $(TARGETS) -o $@

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ): 
	mkdir -p $@

$(BIN):
	mkdir -p $@

clean: $(BIN) $(OBJ)
	rm -rf $(BIN)
	rm -rf $(OBJ)

install: $(BIN)/graphX
	cp $(BIN)/graphX /usr/local/bin

uninstall:
	rm /usr/local/bin/graphX