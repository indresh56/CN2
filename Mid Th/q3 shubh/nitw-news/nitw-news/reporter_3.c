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

    // First message after 10 seconds
    sleep(10);
    char* msg1 = "New protocol update improves network security.\n"
                 "Enhanced encryption algorithms are included.\n"
                 "Update aims to counter new cyber threats.\n"
                 "Network administrators should apply it soon.\n"
                 "Also introduces efficient data transmission.\n"
                 "Expect better overall network performance.\n";
    
    // Send the first message
    send(sock, msg1, strlen(msg1), 0);
    printf("Message 1 sent\n");

    // Second message after another 10 seconds
    sleep(10);
    char* msg2 = "\\dDubai unveils new skyscraper, the tallest in the city.\n"
                 "The building features innovative design and technology.\n"
                 "Construction has boosted local employment.\n"
                 "Dubai's skyline continues to impress globally.\n"
                 "Tourists flock to see the latest architectural marvel.\n"
                 "The city remains a hub for modern advancements.\n";
    
    // Send the second message
    send(sock, msg2, strlen(msg2), 0);
    printf("Message 2 sent\n");

    // Close the socket after sending the messages
    close(sock);
    return 0;
}
