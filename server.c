#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

#include "message.h"
#include "socket.h"

#define NUM_THREAD 4

typedef struct thread_args {
  int client_socket_fd;
} thread_args_t;


void* thread_fn(void* args) {
  thread_args_t* arg = (thread_args_t*) args;
  char* message;
  int client_socket_fd = arg->client_socket_fd;
    // Read a message from the client & Send a message to the client
    do {
      message = receive_message(client_socket_fd);
     
      if (message == NULL) {
        perror("Failed to read message from client");
        exit(EXIT_FAILURE);
      }
     
      if (strcmp(message, "quit\n") == 0) {
        printf("Client exited\n");
        break;
      }

      // counter for the loop
      for (int i = 0; i < strlen(message); i++) {
        message[i] = toupper(message[i]);
      }

      int rc = send_message(client_socket_fd, message);
      if (rc == -1) {
        perror("Failed to send message to client");
        exit(EXIT_FAILURE);
      }
    } while (1);

    // Print the message
    printf("Client sent: %s\n", message);

    // Free the message string
    free(message);

    close(client_socket_fd);

    return NULL;
}

int main() {
  pthread_t threads[NUM_THREAD];
  thread_args_t args[NUM_THREAD];

  // Open a server socket
  unsigned short port = 0;
  int server_socket_fd = server_socket_open(&port);
  if (server_socket_fd == -1) {
    perror("Server socket was not opened");
    exit(EXIT_FAILURE);
  }

  // Start listening for connections, with a maximum of one queued connection
  if (listen(server_socket_fd, 1)) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  int i = 0;

  while (i < NUM_THREAD) {
    printf("Server listening on port %u\n", port);

    // Wait for a client to connect
    int client_socket_fd = server_socket_accept(server_socket_fd);
    if (client_socket_fd == -1) {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }

    printf("Client connected!\n");

    args[i].client_socket_fd = client_socket_fd;
    pthread_create(&threads[i], NULL, thread_fn, &args[i]);
    i++;
  }

  // Close sockets
  close(server_socket_fd);

  return 0;
}