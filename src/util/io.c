#include "dropboxUtil.h"

int readToFile(FILE* pFile, int file_size, int sockid) {
  int bytes_written = 0;
  int bytes_read = 0;

  char buffer[BUFFER_SIZE];

  while(bytes_written < file_size) {
    bytes_read = read(sockid, buffer, BUFFER_SIZE); // recebe arquivo no buffer
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

void write_to_socket(int socket, char* buffer) {
  int a_enviar = BUFFER_SIZE;
  int enviado = 0;
  int pos_buffer = 0;

  while((a_enviar - enviado) > 0) { // enquanto está enviando
    enviado += write(socket, buffer + pos_buffer, a_enviar - enviado);
    pos_buffer += enviado;
  }
}

void read_from_socket(int socket, char* buffer) {
  int a_ler = BUFFER_SIZE;
  int lido = 0;
  int pos_buffer = 0;

  bzero(buffer, BUFFER_SIZE);
  while((a_ler - lido) > 0) {
    lido += read(socket, buffer + pos_buffer, a_ler - lido);
    pos_buffer += lido;
  }
}
