#include "dropboxClient.h"

pthread_t sync_thread;
struct user_info user;
int sockid;
int status;

char buffer[BUFFER_SIZE];

void clearBuffer() {
  int i;
  for(i = 0; i < BUFFER_SIZE; i++){
    buffer[i] = 0;
  }
}

int connect_server (char *host, int port) {

  struct sockaddr_in serverconn;

  /* Create a socket point */
  sockid = socket(AF_INET, SOCK_STREAM, 0);  

  if (sockid < 0) {
      printf("ERROR opening socket\n");
      exit(1);
  }

  bzero((char *) &serverconn, sizeof(serverconn));

  serverconn.sin_family = AF_INET;
  serverconn.sin_port = htons(port);
  serverconn.sin_addr.s_addr = inet_addr(host);

 

  status = connect(sockid, (struct sockaddr *) &serverconn, sizeof(serverconn));
  if(status < 0){
    printf("\nConnection Error\n");
    exit(1);
  }

  bzero(buffer, BUFFER_SIZE);

  strcpy(buffer, user.id);

  // write to socket
  status = write(sockid, buffer, BUFFER_SIZE);
  if (status < 0) {
      printf("ERROR writing to socket\n");
      exit(1);
   }
   
  // read server response 
  bzero(buffer, BUFFER_SIZE);
  status = read(sockid, buffer, BUFFER_SIZE);
  if (status < 0) {
     printf("ERROR reading from socket\n");
     exit(1);
  }
  if(strcmp(buffer, "conectado") == 0){
    printf("\nConnected\n");
    return 1;
  }
  if(strcmp(buffer, "excess devices") == 0){
    printf("\nMuitas conexões simultaneas do mesmo usuário. encerrando...\n");
  }
  
  return 0;
}

void sync_client() {
	// verifica se o diretório sync_dir_<nomeusuario> existe
	// se não, cria pasta e fecha.
	if(!fileExists(user.folder)) {
		if(mkdir(user.folder, 0777) != 0) {
			printf("Error creating user folder '%s'.\n", user.folder);
		}

		return;
	}

	// sincroniza pasta local com o servidor
	// cria thread para manter a sincronização
	int rc;
	if((rc = pthread_create(&sync_thread, NULL, watcher_thread, (void *) user.folder))) {
		printf("Syncronization Thread creation failed: %d\n", rc);
	}

	// se há um conflito entre cópias do mesmo arquivo entre cliente e servidor,
	// apagar arquivo local e trazer do servidor.
		// delete_file, mas apenas local.
		// get_file
}

void send_file(char *file) {
	int file_size = 0;
	int bytes_read = 0;

	FILE* pFile;

	pFile = fopen(file, "rb");
	if(pFile) {
		file_size = getFilesize(pFile);

		if(file_size == 0) {
			fclose(pFile);
			return;
		}

		bzero(buffer, BUFFER_SIZE);
		sprintf(buffer,"%c",WANTS_SEND);
		if( 0 > write(sockid, buffer, BUFFER_SIZE)){
			printf("Error sending the type of the operation!\n");
			return;
		}

		bzero(buffer, BUFFER_SIZE);
		int offset = strlen(file);
		for(;offset>0;offset--)
			if(file[offset] == '/' ){
				offset ++;
				break;
			}

		sprintf(buffer,"/%s",file+offset);
		if(0 > write(sockid, buffer, BUFFER_SIZE)){
			printf("Error sending the name of the file!\n");
			return;
		}

		bzero(buffer, BUFFER_SIZE);
		sprintf(buffer,"%d",file_size);
		if(0 > write(sockid, buffer, BUFFER_SIZE)){
			printf("Error sending the size of the file!\n");
			return;
		}
		printf("envio de arquivo de %d bytes\n",file_size);

		int sent_bytes;
		while(!feof(pFile)) {

				fread(buffer, sizeof(char), BUFFER_SIZE, pFile);
				bytes_read += sizeof(char) * BUFFER_SIZE;

				sent_bytes = write(sockid, buffer, BUFFER_SIZE);
  				if (sent_bytes < 0) {
      				printf("ERROR writing to socket\n");
      				exit(1);
   				}
   				char *buffer_offset = buffer;
   				while(sent_bytes != BUFFER_SIZE){
   					buffer_offset = buffer + sent_bytes;
   					sent_bytes += write(sockid, buffer_offset, BUFFER_SIZE);
   				}

				// enviar buffer para salvar no servidor
		}

		fclose(pFile);
		printf("Arquivo %s enviado.\n", file);
	} else {
		printf("Erro abrindo arquivo %s.\n", file);
	}
}

void get_file(char *file) {
	int bytes_written = 0;

	FILE* pFile;

	pFile = fopen(file, "wb");
	if(pFile) {
		// requisita arquivo file para o servidor
		// recebe buffer do servidor

			bzero(buffer, BUFFER_SIZE);
			sprintf(buffer,"%c",WANTS_RECEIVE);
			if( 0 > write(sockid, buffer, BUFFER_SIZE)){
				printf("Error sending the type of the operation!\n");
				return;
			}

			int offset = strlen(file);
			for(;offset>0;offset--)
				if(file[offset] == '/' ){
					offset ++;
					break;
				}
			bzero(buffer, BUFFER_SIZE);
			sprintf(buffer,"/%s",file+offset);
			if(0 > write(sockid, buffer, BUFFER_SIZE)){
				printf("Error sending the name of the file!\n");
				return;
			}
			sleep(1);
			bzero(buffer, BUFFER_SIZE);
			int bytes_read = read(sockid, buffer, BUFFER_SIZE);
			if(bytes_read < 0){
			   printf("error reading from socket\n");
			   return;
			 }
			 if(strcmp(buffer,FILE_DONT_EXIST) == 0){
			 	printf("arquivo não existe\n");
			 	return;
			 }
			 
			int file_size = atoi(buffer);
			printf("arquivo de %d bytes encontrado\n",file_size);

		int received_bytes = 0;
		while(bytes_written < file_size){

			received_bytes = read(sockid, buffer, BUFFER_SIZE);
  			if (received_bytes < 0) {
      			printf("ERROR reading from socket\n");
      			exit(1);
   			}
   			char *buffer_offset = buffer;
   			while(received_bytes != BUFFER_SIZE){
   				buffer_offset = buffer + received_bytes;
   				received_bytes += read(sockid, buffer_offset, BUFFER_SIZE);
   			}
			bytes_written += received_bytes;

			if(bytes_written < file_size)
				fwrite(buffer, sizeof(char), BUFFER_SIZE, pFile);
			else
				fwrite(buffer, sizeof(char), file_size % BUFFER_SIZE, pFile);
		 }
		fclose(pFile);

		printf("Arquivo %s salvo.\n", file);
	} else {
		printf("Erro abrindo arquivo %s.\n", file);
	}
}

void delete_file(char *file) {
	// avisa servidor para remover arquivo file
	// recebe confirmação de que arquivo foi removido
}

void close_connection() {
	// Fechar a thread de sincronização
	pthread_cancel(sync_thread);
	
	// Fechar conexão com o servidor
	bzero(buffer, BUFFER_SIZE);

	strcpy(buffer, "disconnect");
	status = write(sockid, buffer, BUFFER_SIZE);
  	if (status < 0) {
      		printf("ERROR writing to socket\n");
      		exit(1);
   	}
   	bzero(buffer, BUFFER_SIZE);
  	status = read(sockid, buffer, BUFFER_SIZE);
  	if (status < 0) {
     		printf("ERROR reading from socket\n");
     		exit(1);
  	}
  	if(strcmp(buffer, "disconnected") == 0){
    		printf("\nDesconectado!\n");
		close(sockid);
  	} else{
		printf("Erro ao desconectar!\n");		
	}

      
    
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		puts("Argumentos Insuficientes.");
		puts("Formato esperado: './dropboxClient user endereço porta'");

		exit(0);
	}

	int porta;
	char *endereco;

	// Parsing de argumentos do programa
	if (strlen(argv[1]) <= MAXNAME) {
		strcpy(user.id, argv[1]);
		sprintf(user.folder, "%s/sync_dir_%s", getUserHome(), user.id);
	} else {
		puts("Tamanho máximo de userid foi ultrapassado.");
		printf("Máximo: %d. Inserido: %d.\n", MAXNAME, strlen(argv[1]) <= MAXNAME);

		return MAXNAMESIZE_REACHED;
	}

	endereco = malloc(sizeof(argv[2]));
	strcpy(endereco, argv[2]);

	porta = atoi(argv[3]);

	// Efetua conexão ao servidor
	if ((connect_server(endereco, porta))) {
		// sincroniza diretórios (cliente e servidor)
		sync_client();

		// cria a interface do cliente e espera por comandos
		show_client_interface();
	} else {
    		printf("Conexão ao servidor '%s' na porta '%d' falhou.\n", endereco, porta);
		//close(sockid);
	}
}
