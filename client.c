#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "message.h"
#include "socket.h"

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