#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int news_fd;
    int opt = 1;
    news_fd = socket(AF_INET, SOCK_STREAM, 0); 
    setsockopt(news_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in news_address;
    news_address.sin_family = AF_INET;
    news_address.sin_addr.s_addr = INADDR_ANY;
    news_address.sin_port = htons(9000);
    bind(news_fd, (struct sockaddr*) &news_address, sizeof(news_address));

    listen(news_fd, 5);

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);

        int client_fd = accept(news_fd, (struct sockaddr*) &client_address, &client_address_len);

        char* msg1 = "The city council approves new green spaces.\n"
            "Parks will include walking trails and playgrounds.\n"
            "Environmental benefits include reduced pollution.\n";

        for (int i = 0; i < strlen(msg1); i++) {
            write(client_fd, &msg1[i], 1);
            usleep(40000);
        }

        sleep(5);
        
        char* msg2 = "Local wildlife will also thrive in these areas.\n"
            "Construction is set to begin next month.\n"
            "Community feedback has been overwhelmingly positive.\n";

        for (int i = 0; i < strlen(msg2); i++) {
            write(client_fd, &msg2[i], 1);
            usleep(40000);
        }

        close(client_fd);
    }
}