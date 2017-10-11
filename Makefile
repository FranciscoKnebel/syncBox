CC = gcc
CFLAGS = -Wall -g

HEADERS_DIR = headers/
SRC_DIR = src/
BIN_DIR = bin/

DST_DIR = dst/
CLI_DIR = $(DST_DIR)client/
SVR_DIR = $(DST_DIR)server/

all: util client server

util:	$(SRC_DIR)dropboxUtil.c
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)dropboxUtil.o -I$(HEADERS_DIR) $(SRC_DIR)dropboxUtil.c

client:	$(SRC_DIR)dropboxClient.c util
	$(CC) $(CFLAGS) -o $(CLI_DIR)dropboxClient $(SRC_DIR)dropboxClient.c $(BIN_DIR)dropboxUtil.o -pthread -I$(HEADERS_DIR)

server: $(SRC_DIR)dropboxServer.c server
	$(CC) $(CFLAGS) -o $(SVR_DIR)dropboxServer $(SRC_DIR)dropboxServer.c $(BIN_DIR)dropboxUtil.o -pthread -I$(HEADERS_DIR)

clean:
	rm -f $(BIN_DIR)*.o $(DST_DIR)*.* $(CLI_DIR)dropboxClient $(SVR_DIR)dropboxServer
