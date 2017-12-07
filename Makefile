## Flags pra compilar em produção. Se for compilar para uma arquitetura específica (ou usar em outra), trocar o valor native.
## Quando for usar, remover comentário da linha #CFLAGS += $(PRODUCTIONFLAGS)
PRODUCTIONFLAGS = -march=native -O2 -pipe #-Wextra

CC = gcc
CFLAGS = -Wall -g -std=gnu11
#CFLAGS += $(PRODUCTIONFLAGS)

LFLAGS = -pthread -lssl -lcrypto

HEADERS_DIR = headers/
SRC_DIR = src/
BIN_DIR = bin/

DST_DIR = dst/
CLI_DIR = $(DST_DIR)
SVR_DIR = $(DST_DIR)

TST_SRC_DIR = test/
TST_DST_DIR = $(TST_SRC_DIR)dst/

all: util assets client server
	@echo "All files compiled!"

util:	$(SRC_DIR)dropboxUtil.c
	@echo "\nCompilando módulos utilitários..."
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)dropboxUtil.o -I$(HEADERS_DIR) $(SRC_DIR)dropboxUtil.c $(LFLAGS)
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)util/files.o -I$(HEADERS_DIR) $(SRC_DIR)util/files.c $(LFLAGS)
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)util/io.o -I$(HEADERS_DIR) $(SRC_DIR)util/io.c $(LFLAGS)
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)util/ssl.o -I$(HEADERS_DIR) $(SRC_DIR)util/ssl.c $(LFLAGS)

OBJ_FILES = $(BIN_DIR)dropboxUtil.o $(BIN_DIR)util/files.o $(BIN_DIR)util/io.o $(BIN_DIR)util/ssl.o

ssl:
	@echo "\nCopiando arquivos ssl de teste para pasta de destino..."
	cp *.pem $(DST_DIR)

## CLIENT COMMANDS
client-aux: $(SRC_DIR)client/interface.c $(SRC_DIR)client/commands.c $(SRC_DIR)client/help.c $(SRC_DIR)client/sync.c
	@echo "\nCompilando módulos do cliente..."
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)client/interface.o -I$(HEADERS_DIR) $(SRC_DIR)client/interface.c $(LFLAGS)
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)client/commands.o -I$(HEADERS_DIR) $(SRC_DIR)client/commands.c $(LFLAGS)
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)client/help.o -I$(HEADERS_DIR) $(SRC_DIR)client/help.c $(LFLAGS)
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)client/sync.o -I$(HEADERS_DIR) $(SRC_DIR)client/sync.c $(LFLAGS)

CLIENT_FILES = $(BIN_DIR)client/*.o
client:	$(SRC_DIR)dropboxClient.c util client-aux ssl
	@echo "Linkando objetos e compilando aplicação do cliente."
	$(CC) $(CFLAGS) -o $(CLI_DIR)dropboxClient $(SRC_DIR)dropboxClient.c $(OBJ_FILES) $(CLIENT_FILES) $(LFLAGS) -I$(HEADERS_DIR)

## SERVER COMMANDS
server-aux:
	@echo "\nCompilando módulos do servidor..."
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)server/client.o -I$(HEADERS_DIR) $(SRC_DIR)server/client.c $(LFLAGS)
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)server/commands.o -I$(HEADERS_DIR) $(SRC_DIR)server/commands.c $(LFLAGS)
	$(CC) $(CFLAGS) -c -o $(BIN_DIR)server/sync.o -I$(HEADERS_DIR) $(SRC_DIR)server/sync.c $(LFLAGS)

SERVER_FILES = $(BIN_DIR)server/*.o
server: $(SRC_DIR)dropboxServer.c util server-aux ssl
	@echo "Linkando objetos e compilando aplicação do servidor."
	$(CC) $(CFLAGS) -o $(SVR_DIR)dropboxServer $(SRC_DIR)dropboxServer.c $(OBJ_FILES) $(SERVER_FILES) $(LFLAGS) -I$(HEADERS_DIR)

## TEST
test: test_dropboxUtil test_util
	@echo "All tests finished."

test_util: util
	$(CC) $(CFLAGS) -o $(TST_DST_DIR)util $(TST_SRC_DIR)util.c $(OBJ_FILES) $(LFLAGS) -I$(HEADERS_DIR)
	./$(TST_DST_DIR)util README.md
	./$(TST_DST_DIR)util src/dropboxUtil.c
	./$(TST_DST_DIR)util headers/dropboxClient.h

test_dropboxUtil: util
	$(CC) $(CFLAGS) -o $(TST_DST_DIR)dropboxUtil $(TST_SRC_DIR)dropboxUtil.c $(OBJ_FILES) $(LFLAGS) -I$(HEADERS_DIR)
	./$(TST_DST_DIR)dropboxUtil assets
	@sleep 3
	./$(TST_DST_DIR)dropboxUtil src
	@sleep 3
	./$(TST_DST_DIR)dropboxUtil src/client
	@sleep 3
	./$(TST_DST_DIR)dropboxUtil .

TST_FILES = $(TST_DST_DIR)dropboxUtil $(TST_DST_DIR)util

clean:
	rm -f $(DST_DIR)*.* $(CLI_DIR)dropboxClient $(SVR_DIR)dropboxServer
	rm -f $(OBJ_FILES) $(CLIENT_FILES) $(SERVER_FILES)
	rm -f $(TST_FILES)
