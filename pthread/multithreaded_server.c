#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to handle communication with a client
void *handle_client(void *client_socket) {
    int sock = *(int *)client_socket;
    free(client_socket);  // Free the dynamically allocated socket descriptor
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the received data
        printf("Client: %s\n", buffer);  // Print client message

        // Echo the message back to the client
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("Failed to send message to client");
            close(sock);
            pthread_exit(NULL);
        }
    }

    if (bytes_read == 0) {
        printf("Client disconnected.\n");
    } else if (bytes_read < 0) {
        perror("recv failed");
    }

    close(sock);
    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t thread_id;

    // Create the server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Server socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Bind to all interfaces
    server_addr.sin_port = htons(PORT);        // Set the port

    // Bind the socket to the port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept incoming connections in a loop
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) >= 0) {
        printf("Connection accepted from client.\n");

        // Create a new thread for each client
        new_sock = malloc(sizeof(int));  // Allocate memory for the new socket
        *new_sock = client_socket;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)new_sock) != 0) {
            perror("Failed to create thread");
            free(new_sock);  // Free the memory if thread creation fails
        }

        // Detach the thread so it can be cleaned up after finishing
        pthread_detach(thread_id);
    }

    if (client_socket < 0) {
        perror("Accept failed");
    }

    close(server_socket);
    return 0;
}
