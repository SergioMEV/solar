#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

#include "message.h"
#include "socket.h"

// Dynamic array to store socket_fds
int* clients = NULL;
// Number of clients
int num_clients = 0;

void* thread_fn(void* socket_fd) {
  int client_socket_fd = *(int*)socket_fd;
  char* message;

  // Read a message from the client & send a message to the client
  do {
    message = receive_message(client_socket_fd);

    if (message == NULL) {
      int counter = 0;
      for (int i = 0; i < num_clients; i++) {
        if (clients[i] == client_socket_fd) {
          break;
        }
        counter++;
      }
      for (int i = counter; i < num_clients - 1; i++) {
        clients[i] = clients[i + 1];
      }
      num_clients--;
      clients = (int*)realloc(clients, sizeof(int)*num_clients);
      return NULL;
    }

    if (strcmp(message, "quit\n") == 0) {
      printf("Client exited\n");
      break;
    }

    // Sending message to clients in the network
    for (int i = 0; i < num_clients; i++) {
      // Skip if socket_fd is the same
      if (clients[i] == client_socket_fd) {
          continue;
      }

      if (send_message(clients[i], message) == -1) {
          perror("Failed to send message to client");
          exit(EXIT_FAILURE);
      }
    }
  } while (1);

  // Print the message
  printf("Client sent: %s\n", message);
  // Free the message string
  free(message);
  // Close socket
  close(client_socket_fd);

    return NULL;
}

int main() {
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

  printf("Server listening on port %u\n", port);

  while (1) {
    // Wait for a client to connect
    int client_socket_fd = server_socket_accept(server_socket_fd);
    if (client_socket_fd == -1) {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }

    printf("Client connected!\n");

    // Save client's information
    num_clients++;
    clients = (int*)realloc(clients, sizeof(int)*num_clients);
    clients[num_clients - 1] = client_socket_fd;

    // Create a thread to receive and send messages to peers
    pthread_t server_thread;
    if (pthread_create(&server_thread, NULL, &thread_fn, &client_socket_fd)) {
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
  }

  // Close sockets
  close(server_socket_fd);

  return 0;
}