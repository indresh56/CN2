#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX 512

int is_free = 1;

struct msg_t {
    long msg_type;
    char msg_body[MAX];
} message;

void handler1() {
    is_free = 0;
}

void handler2() {
    is_free = 1;
}

int main() {
    fflush(stdout);
    signal(SIGUSR1, handler1);
    signal(SIGUSR2, handler2);

    int opt = 1;
    int port = 12345;
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(12345);
    bind(sock_fd, (struct sockaddr*) &addr, sizeof(addr));

    key_t key = ftok("msgqfile", 65);
    int msqid = msgget(key, 0666 | IPC_CREAT);

    // char buffer[1024];
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);

    while (1) {
        if (is_free) {
            recvfrom(sock_fd, &message.msg_body, sizeof(message.msg_body), 0, (struct sockaddr*) &client_address, &client_len);
            // msgrcv(msqid, &message, sizeof(message), 1, 0);
            int n = strlen(message.msg_body);

            if (message.msg_body[0] == '\\') {
                int port;
                sscanf(message.msg_body, "\\%d", &port);
                printf("connecting to port %d...\n", port);

                FILE* pid = popen("pgrep news_reader_2", "r");
                char pid_buffer[MAX];
                fgets(pid_buffer, MAX, pid);
                int pid_no = atoi(pid_buffer);
                kill(pid_no, SIGUSR1);

                int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
                struct sockaddr_in server_address;
                server_address.sin_family = AF_INET;
                server_address.sin_addr.s_addr = INADDR_ANY;
                server_address.sin_port = htons(port);
                connect(sock_fd, (struct sockaddr*) &server_address, sizeof(server_address));

                char buffer[1];
                while (read(sock_fd, buffer, 1)) {
                    printf("%c", buffer[0]);
                    fflush(stdout);
                }

                kill(pid_no, SIGUSR2);
                close(sock_fd);
            } 
            
            else {
                for (int i = 0; i < n; i++) {
                    printf("%c", message.msg_body[i]);
                    fflush(stdout);
                    usleep(40000);
                }
            }
            
            printf("....................................................\n");
            sleep(3);
        }
    }
}