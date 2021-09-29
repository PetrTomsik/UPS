CC = gcc
CFLAGS = -Wall -pedantic -ansi
BIN = server
OBJ = server.o game.o message_handler.o player.o stack.o player_db.o lobby.o

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BIN): $(OBJ)
	$(CC) $^ -pthread -o $@