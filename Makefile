HEADERS_DIR = ./headers
SRC_DIR = ./src
BIN_DIR = ./bin
CLI_DIR = ./dropboxClient
SVR_DIR = ./dropboxServer

all: utils client server

utils:	$(SRC_DIR)/dropboxUtil.c 
	$gcc -c $(SRC_DIR)/dropboxUtil.c && mv dropboxUtil.o $(BIN_DIR)

client:	$(SRC_DIR)/dropboxClient.c
	$gcc -o $(CLI_DIR)/dropboxClient $(SRC_DIR)/dropboxClient.c $(BIN_DIR)/dropboxUtil.o -pthread

server: $(SRC_DIR)/dropboxServer.c
	$gcc -o $(SVR_DIR)/dropboxServer $(SRC_DIR)/dropboxServer.c $(BIN_DIR)/dropboxUtil.o -pthread
