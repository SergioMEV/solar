#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "message.h"
#include "socket.h"

void* thread_fn(void* socket_fd) {
  printf("in 1\n");
  int server_socket_fd = *(int*)socket_fd;

  while (1) {
    printf("in 2 %d\n", server_socket_fd);
    // Receive message from clients
    char* message = receive_message(server_socket_fd);
    if (message == NULL){
      perror("message received fail.");
      exit(1);
    }
    printf("in 3\n");
    // Sending message to peers in the network
    // send_message(client_socket_fd, message);

    printf("Server: %s", message);
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

  // Send a message to the server
  char* buffer = NULL;
  char* message;
  size_t buffersize = 0;

  // pthread_t* thread;
  // printf("before thread %d\n", socket_fd);
  // if (pthread_create(thread, NULL, &thread_fn, &socket_fd)) {
  //   perror("pthread_create failed");
  //   exit(EXIT_FAILURE);
  // }

  do {
    getline(&buffer, &buffersize, stdin);

    int rc = send_message(socket_fd, buffer);
    message = receive_message(socket_fd);
    printf("Server: %s", message);

    if (strcmp(buffer, "quit\n") == 0) break;
  } while (1);

  // Close socket
  close(socket_fd);
}