CC = gcc
CFLAGS = -Wall -g

HEADERS_DIR = headers/
SRC_DIR = src/
BIN_DIR = bin/

DST_DIR = dst/
CLI_DIR = $(DST_DIR)client/
SVR_DIR = $(DST_DIR)server/

TST_SRC_DIR= test/
TST_DST_DIR = $(TST_SRC_DIR)dst/

all: util client server
	@echo "All files compiled!"

util:	$(SRC_DIR)dropboxUtil.c $(SRC_DIR)util.c $(SRC_DIR)inotify.c
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)dropboxUtil.o -I$(HEADERS_DIR) $(SRC_DIR)dropboxUtil.c
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)util.o -I$(HEADERS_DIR) $(SRC_DIR)util.c
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)inotify.o -I$(HEADERS_DIR) $(SRC_DIR)inotify.c

OBJ_FILES = $(BIN_DIR)dropboxUtil.o $(BIN_DIR)util.o $(BIN_DIR)inotify.o

## CLIENT COMMANDS
client-aux: $(SRC_DIR)client/interface.c $(SRC_DIR)client/commands.c $(SRC_DIR)client/help.c
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)client/interface.o -I$(HEADERS_DIR) $(SRC_DIR)client/interface.c
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)client/commands.o -I$(HEADERS_DIR) $(SRC_DIR)client/commands.c
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)client/help.o -I$(HEADERS_DIR) $(SRC_DIR)client/help.c

CLIENT_FILES = $(BIN_DIR)client/*.o
client:	$(SRC_DIR)dropboxClient.c util client-aux
	$(CC) $(CFLAGS) -o $(CLI_DIR)dropboxClient $(SRC_DIR)dropboxClient.c $(OBJ_FILES) $(CLIENT_FILES) -pthread -I$(HEADERS_DIR)

## SERVER COMMANDS
server-aux:
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)server/connection.o -I$(HEADERS_DIR) $(SRC_DIR)server/connection.c
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)server/fileUtil.o -I$(HEADERS_DIR) $(SRC_DIR)server/fileUtil.c

SERVER_FILES = $(BIN_DIR)server/*.o
server: $(SRC_DIR)dropboxServer.c util server-aux
	$(CC) $(CFLAGS) -o $(SVR_DIR)dropboxServer $(SRC_DIR)dropboxServer.c $(OBJ_FILES) $(SERVER_FILES) -pthread -I$(HEADERS_DIR)

## TEST
test: test_dropboxUtil test_util test_inotify
	@echo "All tests finished."

test_util: util
	$(CC) $(CFLAGS) -o $(TST_DST_DIR)util $(TST_SRC_DIR)util.c $(OBJ_FILES) -pthread -I$(HEADERS_DIR)
	./$(TST_DST_DIR)util README.md
	./$(TST_DST_DIR)util src/util.c
	./$(TST_DST_DIR)util headers/dropboxClient.h

test_dropboxUtil: util
	$(CC) $(CFLAGS) -o $(TST_DST_DIR)dropboxUtil $(TST_SRC_DIR)dropboxUtil.c $(OBJ_FILES) -pthread -I$(HEADERS_DIR)
	./$(TST_DST_DIR)dropboxUtil assets
	@sleep 3
	./$(TST_DST_DIR)dropboxUtil src
	@sleep 3
	./$(TST_DST_DIR)dropboxUtil src/client
	@sleep 3
	./$(TST_DST_DIR)dropboxUtil .

test_inotify: util
	$(CC) $(CFLAGS) -o $(TST_DST_DIR)inotify $(TST_SRC_DIR)inotify.c $(OBJ_FILES) -pthread -I$(HEADERS_DIR)
	./$(TST_DST_DIR)inotify /home/francisco/sync_dir_1

TST_FILES = $(TST_DST_DIR)/dropboxUtil $(TST_DST_DIR)/util $(TST_DST_DIR)/inotify

clean:
	rm -f $(DST_DIR)*.* $(CLI_DIR)dropboxClient $(SVR_DIR)dropboxServer
	rm -f $(OBJ_FILES) $(CLIENT_FILES) $(SERVER_FILES)
	rm -f $(TST_FILES)
