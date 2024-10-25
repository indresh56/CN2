#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <sys/select.h>
#include <signal.h>
#include <netinet/ip.h>
#include <sys/stat.h>
#include <errno.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/udp.h>
#include <ifaddrs.h>
#include <netdb.h>
#define TCP 1
#define UDP 2
// BSD Sockets
void getmyip(char *ip)
{
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    // Get a linked list of network interfaces
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return;
    }

    // Loop through each network interface
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue; // Skip if there's no address

        // Only consider IPv4 addresses
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            // Convert the network address to a readable format
            if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0)
            {
                // Skip the loopback interface (127.0.0.1)
                if (strcmp(host, "127.0.0.1") != 0)
                {
                    strcpy(ip, host);
                    return;
                }
            }
        }
    }

    // Free the linked list
    freeifaddrs(ifaddr);

    return;
}
struct sockaddr_in baddr_maker(char *ip, int port)
{
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    return addr;
}

int bserver_sock(int port, int protocol)
{
    int sfd;
    if (protocol == TCP)
        sfd = socket(AF_INET, SOCK_STREAM, 0);
    else
        sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0)
    {
        perror("Socket making failed");
    }
    char ip[100];
    getmyip(ip);
    struct sockaddr_in addr = baddr_maker(ip, port);
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        perror("BSD bind failed");
    if (protocol == TCP)
    {
        if (listen(sfd, 5) < 0)
            perror("BSD listen failed");
    }
    return sfd;
}

int bclient_sock(int port, int protocol)
{
    int sfd;
    if (protocol == TCP)
        sfd = socket(AF_INET, SOCK_STREAM, 0);
    else
        sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0)
    {
        perror("Socket making failed");
    }
    char ip[100];
    getmyip(ip);
    struct sockaddr_in addr = baddr_maker(ip, port);
    socklen_t s = sizeof(addr);
    if (protocol == TCP)
    {
        if (connect(sfd, (struct sockaddr *)&addr, s) < 0)
            perror("Client Connect failed");
    }
    else
    {
        if (bind(sfd, (struct sockaddr *)&addr, s) < 0)
            perror("Bind socket UDP failed");
    }
    return sfd;
}

int bclient_addr_sock(struct sockaddr_in addr)
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        perror("Socket making failed");
    }
    socklen_t s = sizeof(addr);
    if (connect(sfd, (struct sockaddr *)&addr, s) < 0)
        perror("Client Connect failed");
    return sfd;
}

void baddr_to_ip_port(struct sockaddr_in addr, int *arr)
{ // arr is a 5 size arr to get n1.n2.n3.n4 n5(port)
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    int t = 0, j = 0;
    strcat(ip, ".");
    for (int i = 0; i < strlen(ip); i++)
    {
        if (ip[i] == '.')
        {
            arr[j] = t;
            t = 0;
            j++;
        }
        else
        {
            t *= 10;
            t += ip[i] - '0';
        }
    }
    arr[j] = ntohs(addr.sin_port);
}
#define ADDRESS "sococket"
int send_fd(int socket, int fd_to_send)
{
    struct msghdr socket_message;
    struct iovec io_vector[1];
    struct cmsghdr *control_message = NULL;
    char message_buffer[1];
    /* storage space needed for an ancillary element with a paylod of length is CMSG_SPACE(sizeof(length)) */
    char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];
    int available_ancillary_element_buffer_space;
    /* at least one vector of one byte must be sent */
    message_buffer[0] = 'F';
    io_vector[0].iov_base = message_buffer;
    io_vector[0].iov_len = 1;
    /* initialize socket message */
    memset(&socket_message, 0, sizeof(struct msghdr));
    socket_message.msg_iov = io_vector;
    socket_message.msg_iovlen = 1;
    /* provide space for the ancillary data */
    available_ancillary_element_buffer_space = CMSG_SPACE(sizeof(int));
    memset(ancillary_element_buffer, 0, available_ancillary_element_buffer_space);
    socket_message.msg_control = ancillary_element_buffer;
    socket_message.msg_controllen = available_ancillary_element_buffer_space;
    /* initialize a single ancillary data element for fd passing */
    control_message = CMSG_FIRSTHDR(&socket_message);
    control_message->cmsg_level = SOL_SOCKET;
    control_message->cmsg_type = SCM_RIGHTS;
    control_message->cmsg_len = CMSG_LEN(sizeof(int));
    *((int *)CMSG_DATA(control_message)) = fd_to_send;
    return sendmsg(socket, &socket_message, 0);
}
int main()
{
    int usfd;
    struct sockaddr_un userv_addr, ucli_addr;
    int userv_len=sizeof(struct sockaddr_un), ucli_len=sizeof(struct sockaddr_un);
    usfd = socket(AF_UNIX, SOCK_STREAM, 0);
    perror("socket");
    bzero(&userv_addr, sizeof(userv_addr));
    userv_addr.sun_family = AF_UNIX;
    strcpy(userv_addr.sun_path, ADDRESS);
    unlink(ADDRESS);
    userv_len = sizeof(userv_addr);
    if (bind(usfd, (struct sockaddr *)&userv_addr, userv_len) == -1)
        perror("server: bind");
    if (listen(usfd, 10) < 0)
    {
        perror("listen");
        exit(1);
    }
    int sfd = bserver_sock(9000, TCP);
    int nsfd[3];
    struct sockaddr_in addr;
    socklen_t s = sizeof(addr);
    for (int i = 0; i < 3; i++)
    {
        nsfd[i] = accept(sfd, (struct sockaddr *)&addr, &s);
    }
    for (int i = 0; i < 3; i++)
    {
        char buf[100];
        read(nsfd[i], buf, 100);
        int n = atoi(buf);
        for (int j = 0; j < n; j++)
        {
            sprintf(buf, "%d %d %d %d", rand() % 100, rand() % 100, rand() % 100, rand() % 100);
            printf("%s\n",buf);
            send(nsfd[i], buf, 100, 0);
        }
    }
    int nufd;
    if ((nufd = accept(usfd, (struct sockaddr *)&ucli_addr, &ucli_len)) < 0)
    {
        perror("accept");
        exit(1);
    }
    close(usfd);
    for (int i = 0; i < 3; i++){
        send_fd(nufd, nsfd[i]);
    }
    close(nufd);
}