#include "server_connection.h"

int keep_service = 1;



void hq(int *client_socket){
/*
escalonador, a ideia é que diversas threads utilizem está função para decidir o que fazer,
cuidado deve ser para que somente haja uma escrita por arquivo...
*/
  int n,i=0;
  char mission[MAXNAME],user_name[MAXNAME],current;

  printf("In headquarters - client %d\n",*client_socket);

  //leitura do usuário
  do{
    n = read(*client_socket,&current,1);
    if(n>0)
      user_name[i++] = current;
  }while(current!='$');
  user_name[i-1] = '\0';

  //fazer a leitura dos meta dados do usuário, criar caso não existam

  //leitura da operação
  do{
    n = read(*client_socket,&current,1);
    if(n>0)
      mission[i++] = current;
  }while(current!='$');
  mission[i-1] = '\0';

  if(strcmp(SYNC_REQUEST,mission)==0) {
    //sync_server();
  } else if(strcmp(SEND_REQUEST,mission)==0) {
    //receive_file();
  } else if(strcmp(RECEIVE_REQUEST,mission)==0) {
    //send_file();
  } // faltava argumentos, só pra conseguir compilar o server
}


int receive_connection(void){
/*
seria básicamente a main do servidor! recebe conexão, gera thread soldado, repete!
*/
  //para sockets
  int sockfd, newsockfd;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  //para threads
  pthread_t soldier[MAX_CONNECTIONS];
  int last_soldier = 0;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    perror("ERROR opening socket");
    return 1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(serv_addr.sin_zero), 8);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    perror("ERROR on binding");
    return 2;
  }

  listen(sockfd, MAX_CONNECTIONS);

  clilen = sizeof(struct sockaddr_in);

  //loop principal de gera thread para conexão
  while(keep_service){
    if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) <= 0)
      perror("ERROR on accept");

    puts("loop de atendimento");
    //fica tentando encontrar trabalhador livre

    while(pthread_tryjoin_np(soldier[last_soldier],NULL)!=0){
      last_soldier = ++last_soldier % MAX_CONNECTIONS;
    }

      pthread_create(&soldier[last_soldier], NULL, (void *)hq, &newsockfd);

  }
  return 0;
}
