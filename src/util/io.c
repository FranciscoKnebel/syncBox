#include "dropboxUtil.h"

int read_to_file(FILE* pFile, int file_size, SSL *ssl) {
  int bytes_written = 0;
  int bytes_read = 0;

  char buffer[BUFFER_SIZE];

  while(bytes_written < file_size) {
    bytes_read = read_from_socket(ssl, buffer); // recebe arquivo no buffer
    if (bytes_read < 0) {
      DEBUG_PRINT("ERROR reading from socket\n");
    }

    if(bytes_read > (file_size - bytes_written)) {
      fwrite(buffer, sizeof(char), (file_size - bytes_written), pFile);
      bytes_written += sizeof(char) * (file_size - bytes_written);
    } else {
      fwrite(buffer, sizeof(char), bytes_read, pFile);
      bytes_written += sizeof(char) * bytes_read;
    }
    // DEBUG_PRINT("leu buffer %d bytes - Total: %d / Escritos: %d / Sobrando: %d\n", bytes_read, file_size, bytes_written, (file_size - bytes_written));
  }

  return bytes_written;
}

int write_to_socket(SSL *ssl, char* buffer) {
  int a_enviar = BUFFER_SIZE;
  int enviado = 0;
  int pos_buffer = 0;
  char answer[50];

  while((a_enviar - enviado) > 0) { // enquanto está enviando
    enviado += SSL_write(ssl, buffer + pos_buffer, a_enviar - enviado);
    // SSL_read(ssl, answer, 50);
    // DEBUG_PRINT("Recebido: %s\n", answer);
    // if(strcmp("lido!", answer) != 0){
    //   DEBUG_PRINT("Não leu!\n");
    // }
    pos_buffer += enviado;
  }

  return enviado;
}

int read_from_socket(SSL *ssl, char* buffer) {
  int a_ler = BUFFER_SIZE;
  int lido = 0;
  int pos_buffer = 0;
  char answer[50] = "lido!";

  bzero(buffer, BUFFER_SIZE);
  while((a_ler - lido) > 0) {
    lido += SSL_read(ssl, buffer + pos_buffer, a_ler - lido);
    // SSL_write(ssl, answer, 50);
    pos_buffer += lido;
  }

  return lido;
}
