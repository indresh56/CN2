#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/stat.h> 
#include <sys/socket.h>
#include <sys/un.h>
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
#define REPORTERS 3
#define FIFO_NAME "news_fifo"
#define SOCK_PATH "fd_socket"

struct msg_t {
    long msg_type;
    char msg_body[MAX];
} message;

// Function to send the file descriptor over a Unix domain socket
void send_fd(int socket, int fd_to_send) {
    struct msghdr message = {0};
    struct cmsghdr *control_message = NULL;
    char buffer[CMSG_SPACE(sizeof(int))];
    struct iovec io = { .iov_base = "FD", .iov_len = 2 };

    message.msg_iov = &io;
    message.msg_iovlen = 1;

    if (fd_to_send != -1) {
        message.msg_control = buffer;
        message.msg_controllen = CMSG_SPACE(sizeof(int));

        control_message = CMSG_FIRSTHDR(&message);
        control_message->cmsg_level = SOL_SOCKET;
        control_message->cmsg_type = SCM_RIGHTS;
        control_message->cmsg_len = CMSG_LEN(sizeof(int));

        *((int *) CMSG_DATA(control_message)) = fd_to_send;
    }

    if (sendmsg(socket, &message, 0) == -1) {
        perror("Failed to send file descriptor");
        exit(1);
    }
}

int main() {
    int nr1_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(12345);
    server_address.sin_addr.s_addr = INADDR_ANY;


    int nr2_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    struct sockaddr_in server_address2;
    server_address2.sin_family = AF_INET;
    server_address2.sin_port = htons(12346);
    server_address2.sin_addr.s_addr = INADDR_ANY;
    int curr = 1;

    mkfifo(FIFO_NAME, 0666);
    int fifo_fd = open(FIFO_NAME, O_RDWR);  // Open in read-write mode

    key_t key = ftok("msgqfile", 65);
    int msqid = msgget(key, 0666 | IPC_CREAT);

    struct pollfd pfds[REPORTERS];

    FILE* f1 = popen("./report_desk", "r");
    int fd1 = fileno(f1);
    pfds[0].fd = fd1;
    pfds[0].events = POLLIN;

    FILE* f2 = popen("./reporter_6", "r");
    int fd2 = fileno(f2);
    pfds[1].fd = fd2;
    pfds[1].events = POLLIN;

    FILE* f3 = popen("./reporter_7", "r");
    int fd3 = fileno(f3);
    pfds[2].fd = fd3;
    pfds[2].events = POLLIN;

    // Set up Unix domain socket for FD passing
    int sock_fd;
    struct sockaddr_un addr;
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCK_PATH);
    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sock_fd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    // Accept connection from document writer to send the FD
    printf("Waiting for document writer to connect...\n");
    int doc_writer_fd = accept(sock_fd, NULL, NULL);
    if (doc_writer_fd == -1) {
        perror("accept");
        exit(1);
    }

    printf("Document writer connected. Sending FIFO file descriptor...\n");
    send_fd(doc_writer_fd, fifo_fd);  // Send FIFO FD to document writer

    // Continue reading from reporters and handling messages
    printf("going in\n");
    while (1) {
        poll(pfds, REPORTERS, -1);
        for (int i = 0; i < REPORTERS; i++) {
            if (pfds[i].revents & POLLIN) {
                // printf("hello\n");
                // printf("News received from reporter %d\n", i+1);
                message.msg_type = curr;
                read(pfds[i].fd, message.msg_body, MAX);
                if (message.msg_body[0] == '\\' && message.msg_body[1] == 'd') {
                    int n = strlen(message.msg_body) - 2;
                    memmove(message.msg_body, message.msg_body + 2, n);
                    write(fifo_fd, message.msg_body, n);
                    memset(message.msg_body, 0, MAX);
                    printf("News sent to document editor\n");
                } else {
                    // msgsnd(msqid, &message, sizeof(message), 0);
                    if (curr == 1) sendto(nr1_fd, message.msg_body, strlen(message.msg_body), 0, &server_address, sizeof(server_address));
                    else sendto(nr2_fd, message.msg_body, strlen(message.msg_body), 0, &server_address2, sizeof(server_address2));
                    memset(message.msg_body, 0, MAX);
                    printf("News sent to news reader %d\n", curr);
                    curr = (curr == 1)? 2 : 1;
                }
            }
        }
    }

    close(sock_fd);
    close(doc_writer_fd);
}
