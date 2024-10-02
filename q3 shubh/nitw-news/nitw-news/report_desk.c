#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define PORT 7000
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10 // Maximum number of clients to handle

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    struct pollfd fds[MAX_CLIENTS + 1]; // To monitor server socket + client sockets
    int client_socket[MAX_CLIENTS] = {0}; // Keep track of client sockets
    int activity, i, sd;

    // Creating socket file descriptor for server
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Setting up the address structure for the server
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the given IP and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // printf("Server started. Listening on port %d...\n", PORT);

    // Initialize the pollfd structure
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;  // Monitor server_fd for incoming connections

    for (i = 1; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;  // Mark all client sockets as unused
    }

    while (1) {
        // Poll to monitor sockets
        activity = poll(fds, MAX_CLIENTS + 1, -1); // Infinite timeout

        if (activity < 0) {
            perror("Poll error");
            exit(EXIT_FAILURE);
        }

        // Check if there's a new connection on the server socket
        if (fds[0].revents & POLLIN) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("Accept error");
                exit(EXIT_FAILURE);
            }

            // printf("New client connected: socket fd = %d\n", new_socket);

            // Add new socket to pollfd array
            for (i = 1; i <= MAX_CLIENTS; i++) {
                if (fds[i].fd == -1) {
                    fds[i].fd = new_socket;
                    fds[i].events = POLLIN;  // Monitor for incoming data
                    break;
                }
            }

            if (i > MAX_CLIENTS) {
                // printf("Max clients reached. Closing new connection.\n");
                close(new_socket);
            }
        }

        // Check all client sockets for incoming data
        for (i = 1; i <= MAX_CLIENTS; i++) {
            sd = fds[i].fd;

            if (sd != -1 && (fds[i].revents & POLLIN)) {
                // Client sent some data, read it
                memset(buffer, 0, BUFFER_SIZE);
                valread = read(sd, buffer, BUFFER_SIZE);

                if (valread == 0) {
                    // Client disconnected
                    // printf("Client disconnected: socket fd = %d\n", sd);
                    close(sd);
                    fds[i].fd = -1;  // Mark the slot as available
                } else {
                    // Print the message from the client
                    printf("%s\n", buffer);
                    fflush(stdout);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
