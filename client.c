#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "message.h"
#include "socket.h"
#include "error_codes.h"

#define MAX_USERNAME_LENGTH 20

char *username;

void *server_listener_thread_fn(void *server_socket_fd_void)
{
  int server_socket_fd = *(int *)server_socket_fd_void;
  while (1)
  {
    // Receive message from the server
    char *message = receive_message(server_socket_fd);
    if (message == NULL)
    {
      perror("message received fail.");
      exit(1);
    }

    printf("Server: %s", message);
    free(message);
  }
  return NULL;
}

int main(int argc, char **argv)
{
  if (argc != 4)
  {
    fprintf(stderr, "Usage: %s <server name> <port> <user name>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Read command line arguments
  char *server_name = argv[1];
  unsigned short port = atoi(argv[2]);
  username = (char *)malloc(sizeof(char) * MAX_USERNAME_LENGTH);
  strcpy(username, argv[3]);

  // Connect to the server
  int server_socket_fd = socket_connect(server_name, port);
  if (server_socket_fd == -1)
  {
    perror("Failed to connect");
    exit(EXIT_FAILURE);
  }

  // send username to the server
  if (send_message(server_socket_fd, username) == -1)
  {
    perror("Failed to send message to the server");
    exit(EXIT_FAILURE);
  }
  char *rc_connection = receive_message(server_socket_fd);
  if (strcmp(rc_connection, ERROR_CODE_DUPLICATE_USER_NAME) == 0)
  {
    perror("The user name is already taken. Please conect with a different user name.\n");
    exit(1);
  }
  free(rc_connection);
  pthread_t server_listener_thread;
  if (pthread_create(&server_listener_thread, NULL, server_listener_thread_fn, &server_socket_fd))
  {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Send a message to the server
  char *buffer_get_line = NULL;
  size_t buffer_get_line_size = 0;
  while (1)
  {
    getline(&buffer_get_line, &buffer_get_line_size, stdin);
    buffer_get_line[buffer_get_line_size - 1] = '\0';
    if (send_message(server_socket_fd, buffer_get_line) == -1)
    {
      perror("Failed to send message to the server");
      exit(EXIT_FAILURE);
    }
  }
  free(buffer_get_line);
  // Close socket
  close(server_socket_fd);
}