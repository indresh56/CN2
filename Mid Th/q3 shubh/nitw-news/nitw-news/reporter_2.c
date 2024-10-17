#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 7000
#define SERVER_IP "127.0.0.1"  // Assuming server is running locally
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    
    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported\n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return -1;
    }

    // First message after 5 seconds
    // sleep(5);
    char* msg1 = "\\dIndia and Australia face off in the cricket final.\n"
                 "India scored 280 in their innings.\n"
                 "Australia needs 281 to win the match.\n"
                 "Star players from both sides are in top form.\n"
                 "The final is expected to be a thrilling contest.\n"
                 "Fans from both nations are eagerly watching.\n";
    
    // Send the first message
    send(sock, msg1, strlen(msg1), 0);
    printf("Message 1 sent\n");

    // Second message after 10 seconds
    sleep(10);
    char* msg2 = "NASA's latest mission successfully lands rover on Mars.\n"
                 "The rover will explore the planet's surface for signs of life.\n"
                 "High-resolution images and data will be sent back to Earth.\n"
                 "The mission aims to uncover Mars' geological history.\n"
                 "Scientists are excited about the potential discoveries.\n"
                 "This marks a significant milestone in space exploration.\n";
    
    // Send the second message
    send(sock, msg2, strlen(msg2), 0);
    printf("Message 2 sent\n");

    // Close the socket after sending the messages
    close(sock);
    return 0;
}
