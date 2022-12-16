#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "message.h"
#include "socket.h"
#include "status_codes.h"
#include "file_system.h"
#include "query_util.h"
#include "ui.h"
#include "constants.h"

char *username;
int server_socket_fd;
file_content_t *file_content;

void *server_listener_thread_fn(void *ptr)
{
  while (1)
  {
    // Receive message from the server
    char *query = receive_message(server_socket_fd);
    if (query == NULL)
    {
      perror("message received fail.");
      exit(1);
    }

    if (strcmp(query, REQUEST_ACCEPTED) == 0)
    {
      file_content->is_blocked = REQUEST_ACCEPTED;
    }
    else if (strcmp(query, REQUEST_DENIED) == 0)
    {
      file_content->is_blocked = REQUEST_DENIED;
    }
    else
    {
      char *query_sep_ptr = query;
      char *line_index_str = strsep(&query_sep_ptr, QUERY_SEPERATOR);
      int line_index = atoi(line_index_str);
      char *action_str = strsep(&query_sep_ptr, QUERY_SEPERATOR);
      char action = action_str[0];
      char *user_name = strsep(&query_sep_ptr, QUERY_SEPERATOR);
      char *modified_line = query_sep_ptr;
      process_query(file_content, user_name, line_index, action, modified_line);

      // Adding to log
      char *log_user_name = malloc(sizeof(char) * (strlen(user_name) + 1));
      char *log_modified_line = malloc(sizeof(char) * (strlen(modified_line) + 1));

      strcpy(log_user_name, user_name);
      strcpy(log_modified_line, modified_line);

      add_to_log(file_content, log_user_name, log_modified_line, line_index, action);
    }

    free(query);
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
  server_socket_fd = socket_connect(server_name, port);
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
  char *connection_status = receive_message(server_socket_fd);
  if (strcmp(connection_status, ERROR_CODE_DUPLICATE_USER_NAME) == 0)
  {
    perror("The user name is already taken. Please conect with a different user name.\n");
    exit(1);
  }
  free(connection_status);
  char *file_name = receive_message(server_socket_fd);
  char *file_content_str = receive_message(server_socket_fd);
  file_content = init_file_content_with_text(file_name, server_socket_fd, username, file_content_str);
  free(file_name);
  free(file_content_str);

  // Create a thread to listen to server messages
  pthread_t server_listener_thread;
  if (pthread_create(&server_listener_thread, NULL, server_listener_thread_fn, NULL))
  {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Create a thread for displaying ui
  pthread_t ui_thread;

  if (pthread_create(&ui_thread, NULL, ui_thread_handler, (void *)file_content))
  {
    perror("Couldn't create display thread:");
    exit(2);
  }

  free(username);

  if (pthread_join(ui_thread, NULL))
  {
    perror("Couldn't join display thread");
    exit(2);
  }
}