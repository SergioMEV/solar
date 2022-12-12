#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "message.h"
#include "socket.h"

void* server_listener_thread_fn(void* server_socket_fd_void) {
  int server_socket_fd = *(int*)server_socket_fd_void;
  while (1) {
    // Receive message from clients
    char* message = receive_message(server_socket_fd);
    if (message == NULL){
      perror("message received fail.");
      exit(1);
    }

    printf("Server: %s", message);
    free(message);
  }
  return NULL;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <server name> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Read command line arguments
  char* server_name = argv[1];
  unsigned short port = atoi(argv[2]);

  // Connect to the server
  int socket_fd = socket_connect(server_name, port);
  if (socket_fd == -1) {
    perror("Failed to connect");
    exit(EXIT_FAILURE);
  }

  pthread_t server_listener_thread;
  if (pthread_create(&server_listener_thread, NULL, server_listener_thread_fn, &socket_fd)) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Send a message to the server
  char* buffer = NULL;
  size_t buffersize = 0;
  do {
    getline(&buffer, &buffersize, stdin);
    buffer[buffersize-1] = '\n';
    int rc = send_message(socket_fd, buffer);
    if (rc == -1) {
      perror("Failed to send message to the server");
      exit(EXIT_FAILURE);
    }
  } while (1);
  free(buffer);
  // Close socket
  close(socket_fd);
}