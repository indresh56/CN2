#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 7000

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char* msg1 = "Israeli airstrikes have killed 274 in Hezbollah areas.\n"
                 "The military warns to avoid Hezbollah sites.\n"
                 "Strikes will continue for the foreseeable future.\n"
                 "Explosions around Baalbek caused extensive damage.\n"
                 "Smoke has been rising from the city.\n"
                 "Hezbollah promises retaliation amid rising tensions.\n"
                 "International calls for peace are growing.\n";
    char* msg2 = "\\9000";

    // Creating socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    // Setting up the address structure for the server
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address or Address not supported \n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Send first message
    send(sock, msg1, strlen(msg1), 0);
    printf("Message 1 sent\n");

    // Wait for 4 seconds
    sleep(4);

    // Send second message
    send(sock, msg2, strlen(msg2), 0);
    printf("Message 2 sent\n");

    // Close the socket
    close(sock);
    return 0;
}
