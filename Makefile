CC = gcc
CFLAGS = -Wall -g

HEADERS_DIR = headers/
SRC_DIR = src/
BIN_DIR = bin/

DST_DIR = dst/
CLI_DIR = $(DST_DIR)client/
SVR_DIR = $(DST_DIR)server/

all: util client server

util:	$(SRC_DIR)dropboxUtil.c $(SRC_DIR)util.c
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)util.o -I$(HEADERS_DIR) $(SRC_DIR)util.c
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)dropboxUtil.o -I$(HEADERS_DIR) $(SRC_DIR)dropboxUtil.c

OBJ_FILES = $(BIN_DIR)dropboxUtil.o $(BIN_DIR)util.o

## CLIENT COMMANDS
client-aux: $(SRC_DIR)client/interface.c
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)client/interface.o -I$(HEADERS_DIR) $(SRC_DIR)client/interface.c
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)client/commands.o -I$(HEADERS_DIR) $(SRC_DIR)client/commands.c

CLIENT_FILES = $(BIN_DIR)client/interface.o $(BIN_DIR)client/commands.o
client:	$(SRC_DIR)dropboxClient.c util client-aux
	$(CC) $(CFLAGS) -o $(CLI_DIR)dropboxClient $(SRC_DIR)dropboxClient.c $(OBJ_FILES) $(CLIENT_FILES) -pthread -I$(HEADERS_DIR)

## SERVER COMMANDS
server-aux:
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)server/connection.o -I$(HEADERS_DIR) $(SRC_DIR)server/connection.c
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)server/fileUtil.o -I$(HEADERS_DIR) $(SRC_DIR)server/fileUtil.c

SERVER_FILES = $(BIN_DIR)server/connection.o $(BIN_DIR)server/fileUtil.o
server: $(SRC_DIR)dropboxServer.c util server-aux
	$(CC) $(CFLAGS) -o $(SVR_DIR)dropboxServer $(SRC_DIR)dropboxServer.c $(OBJ_FILES) $(SERVER_FILES) -pthread -I$(HEADERS_DIR)

clean:
	rm -f $(DST_DIR)*.* $(CLI_DIR)dropboxClient $(SVR_DIR)dropboxServer
	rm -f $(OBJ_FILES) $(CLIENT_FILES) $(SERVER_FILES)
