#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

#include "message.h"
#include "socket.h"
#include "user_info_utils.h"
#include "file_system.h"
#include "status_codes.h"
#include "constants.h"
// #include "query_util.h"

// user information (fd and username)
user_infos_array_t *user_infos_struct;
file_content_t *file_content;

void *client_listener_thread(void *user_info_void)
{
  user_info_t *user_info = (user_info_t *)user_info_void;
  char *query;
  // Read a message from the client & send a message to the client
  while (1)
  {
    // one of the client is dead, remove it
    if ((query = receive_message(user_info->fd)) == NULL)
    {
      user_infos_remove_user(user_infos_struct, user_info);
      printf("User <%s> leaves the session, remaininng number of users: %zu\n", user_info->user_name, user_infos_struct->size);
      break;
    }

    printf("User <%s> sends the query: <%s>\n", user_info->user_name, query);

    // change file_content on the server
    char *query_sep_cpy = malloc(sizeof(char) * (strlen(query)+1));
    strcpy(query_sep_cpy, query);
    char *query_sep_ptr = query_sep_cpy;
    char *line_index_str = strsep(&query_sep_ptr, QUERY_SEPERATOR);
    int line_index = atoi(line_index_str);
    char *action_str = strsep(&query_sep_ptr, QUERY_SEPERATOR);
    char action = action_str[0];
    char *user_name = strsep(&query_sep_ptr, QUERY_SEPERATOR);
    char *modified_line = query_sep_ptr;
    process_query(file_content, user_name, line_index, action, modified_line);
    free(query_sep_cpy);
    // Sending message to clients in the network
    for (int index = 0; index < user_infos_struct->size; index++)
    {
      // Skip if socket_fd is the same
      if (user_infos_struct->user_infos[index] == user_info)
      {
        continue;
      }

      if (send_message(user_infos_struct->user_infos[index]->fd, query) == -1)
      {
        perror("Failed to send message to client");
        exit(EXIT_FAILURE);
      }
    }
    free(query);
  }
  
  // Close socket
  close(user_info->fd);
  user_info_destroy(user_info);

  return NULL;
}

int main()
{
  // Open a server socket
  unsigned short port = 0;
  int server_socket_fd = server_socket_open(&port);
  if (server_socket_fd == -1)
  {
    perror("Server socket was not opened");
    exit(EXIT_FAILURE);
  }

  // Start listening for connections, with a maximum of one queued connection
  if (listen(server_socket_fd, 1))
  {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }
  printf("Server listening on port %u\n", port);

  user_infos_struct = user_infos_array_init();

  // TODO: make file name a user input
  char *file_name = malloc(sizeof(char) * MAX_FILE_NAME_LENGTH);
  strcpy(file_name, "Archive/f1.txt");
  FILE *fptr = open_file_read_mode(file_name);
  // TODO: user_name hard coded!
  file_content = init_file_content_with_file(file_name, server_socket_fd, "server", fptr);
  fclose(fptr);
  free(file_name);

  while (1)
  {
    // Wait for a client to connect
    int client_socket_fd = server_socket_accept(server_socket_fd);
    if (client_socket_fd == -1)
    {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }

    char *user_name = receive_message(client_socket_fd);
    if (is_duplicate_user_name(user_infos_struct, user_name))
    {
      printf("Rejected connection from user <%s> because of duplicate user name\n", user_name);
      // duplicate user name detected
      if (send_message(client_socket_fd, ERROR_CODE_DUPLICATE_USER_NAME) == -1)
      {
        perror("Failed to terminate client connection");
        exit(EXIT_FAILURE);
      }
      continue;
    }

    if (send_message(client_socket_fd, SUCCESS_CODE_CONNECTION_SUCCESS) == -1)
    {
      perror("Failed to send success code to the client");
      exit(EXIT_FAILURE);
    }
    if (send_message(client_socket_fd, file_content->file_name) == -1)
    {
      perror("Failed to send file name to the client");
      exit(EXIT_FAILURE);
    }
    char *file_content_str = file_content_to_string(file_content);
    if (send_message(client_socket_fd, file_content_str) == -1)
    {
      perror("Failed to send file content string to the client");
      exit(EXIT_FAILURE);
    }
    free(file_content_str);

    // Create a user info struct for the new user
    user_info_t *user_info = user_infos_init();
    user_info->user_name = user_name;
    user_info->fd = client_socket_fd;

    user_infos_add_user(user_infos_struct, user_info);

    printf("User <%s> joined the session! Current number of users: %zu\n", user_name, user_infos_struct->size);

    // Create a thread to receive and send messages to peers
    pthread_t server_thread;
    if (pthread_create(&server_thread, NULL, &client_listener_thread, (void *)user_info))
    {
      perror("Createing thread for client listener failed");
      exit(EXIT_FAILURE);
    }
  }

  user_infos_array_destroy(user_infos_struct);
  // Close sockets
  close(server_socket_fd);

  // TODO close all clients' fd 

  return 0;
}

// TODOs: 
//   1. export ui support 
//   2. server need to find a way to keep track of who owns the line 
//   3. be careful about racing condition 
// Warning: no way to queue the changes. 