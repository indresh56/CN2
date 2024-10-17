#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/socket.h>
#include <sys/un.h>

#define MAX 512
#define FIFO_NAME "news_fifo"
#define SOCK_PATH "fd_socket"

// Function to receive the file descriptor from the Unix domain socket
int recv_fd(int socket) {
    struct msghdr message = {0};
    struct cmsghdr *control_message = NULL;
    char buffer[CMSG_SPACE(sizeof(int))];
    struct iovec io = { .iov_base = buffer, .iov_len = 1 };
    message.msg_iov = &io;
    message.msg_iovlen = 1;

    message.msg_control = buffer;
    message.msg_controllen = CMSG_SPACE(sizeof(int));

    if (recvmsg(socket, &message, 0) == -1) {
        perror("Failed to receive file descriptor");
        exit(1);
    }

    control_message = CMSG_FIRSTHDR(&message);
    int fd = *((int *) CMSG_DATA(control_message));
    return fd;
}

int main() {
    // Connect to the editor's Unix domain socket to receive the FD
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

    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        exit(1);
    }

    // Receive FIFO FD
    int fifo_fd = recv_fd(sock_fd);
    printf("Received FIFO file descriptor: %d\n", fifo_fd);

    // Write the received news from FIFO to the file
    int file_fd = open("news.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    char buffer[MAX];
    char* sep = "....................................................\n";
    int size;
    while ((size = read(fifo_fd, buffer, MAX)) > 0) {
        write(file_fd, buffer, size);
        write(file_fd, sep, strlen(sep));
        memset(buffer, 0, MAX);
        printf("Wrote the message in file\n");
    }

    close(fifo_fd);
    close(file_fd);
    close(sock_fd);
}
