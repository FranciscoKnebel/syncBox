#include "server_connection.h"
#include <pthread.h>

int keep_service = 1;

int receive_connection(){

  //para sockets
  int sockfd, newsockfd;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  //para threads
  pthread_t worker[MAX_CONNECTIONS];
  int last_worker = 0;

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

  
  while(keep_service){
    if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
      perror("ERROR on accept");

    
    //fica tentando encontrar trabalhador livre
    while(pthead_tryjoin_np(worker[last_worker],NULL)!=0){
      (last_worker++)%=MAX_CONNECTIONS;
    }

    if(ntohs(cli_addr.sin_port) == SEND_REQUEST_PORT){
      pthread_create(&worker[last_worker], NULL, receive_file, &newsockfd);
    }
    else{
      pthread_create(&worker[last_worker], NULL, send_file, &newsockfd);
    }

  }
  return 0;
}
