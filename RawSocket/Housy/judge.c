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
int recv_fd(int socket)
{
    int sent_fd, available_ancillary_element_buffer_space;
    struct msghdr socket_message;
    struct iovec io_vector[1];
    struct cmsghdr *control_message = NULL;
    char message_buffer[1];
    char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];
    /* start clean */
    memset(&socket_message, 0, sizeof(struct msghdr));
    memset(ancillary_element_buffer, 0, CMSG_SPACE(sizeof(int)));
    /* setup a place to fill in message contents */
    io_vector[0].iov_base = message_buffer;
    io_vector[0].iov_len = 1;
    socket_message.msg_iov = io_vector;
    socket_message.msg_iovlen = 1;
    /* provide space for the ancillary data */
    socket_message.msg_control = ancillary_element_buffer;
    socket_message.msg_controllen = CMSG_SPACE(sizeof(int));
    if (recvmsg(socket, &socket_message, MSG_CMSG_CLOEXEC) < 0)
        return -1;
    if (message_buffer[0] != 'F')
    {
        /* this did not originate from the above function */
        return -1;
    }
    if ((socket_message.msg_flags & MSG_CTRUNC) == MSG_CTRUNC)
    {
        /* we did not provide enough space for the ancillary element array */
        return -1;
    }
    /* iterate ancillary elements */
    for (control_message = CMSG_FIRSTHDR(&socket_message);
         control_message != NULL;
         control_message = CMSG_NXTHDR(&socket_message, control_message))
    {
        if ((control_message->cmsg_level == SOL_SOCKET) &&
            (control_message->cmsg_type == SCM_RIGHTS))
        {
            sent_fd = *((int *)CMSG_DATA(control_message));
            return sent_fd;
        }
    }
    return -1;
}
int main()
{
    int nsfd[3];
    int b[3] = {0, 0, 0};
    int usfd;
    struct sockaddr_un userv_addr;
    int userv_len;
    // Create Unix domain socket
    usfd = socket(AF_UNIX, SOCK_STREAM, 0); // Change to SOCK_STREAM
    if (usfd == -1)
    {
        perror("socket");
        exit(1);
    }
    bzero(&userv_addr, sizeof(userv_addr));
    userv_addr.sun_family = AF_UNIX;
    strcpy(userv_addr.sun_path, ADDRESS);
    userv_len = sizeof(userv_addr);
    // Connect to the server
    if (connect(usfd, (struct sockaddr *)&userv_addr, userv_len) < 0)
    {
        perror("connect");
        exit(1);
    }
    for (int i = 0; i < 3; i++){
        nsfd[i] = recv_fd(usfd);
        printf("%d ",nsfd[i]);
    }
    printf("\n");
    int rsfd = socket(AF_INET, SOCK_RAW, 147);
    if(rsfd<0)
        perror("raw socket fail: ");
    char packet[100];
    sprintf(packet, "%d %d %d %d", rand() % 100, rand() % 100, rand() % 100, rand() % 100);
    printf("%s\n", packet);
    struct sockaddr_in addr;
    socklen_t s = sizeof(addr);
    if(getpeername(nsfd[0], (struct sockaddr *)&addr, &s)<0)
        perror("getpeername: ");
    if(sendto(rsfd, packet, 100, 0, (struct sockaddr *)&addr, s)<0)
        perror("sendto: ");
    for (int i = 0; i < 3; i++)
    {
        if(recv(nsfd[i], packet, 100, 0)<0)
            perror("recv result: ");
        if (packet[0] == 'W')
            b[i] = 1;
    }
    packet[0] = '\0';
    for (int i = 0; i < 3; i++)
    {
        if (b[i] == 1)
            strcat(packet, "W");
        else
            strcat(packet, "L");
    }
    printf("%s\n",packet);
    sendto(rsfd, packet, 100, 0, (struct sockaddr *)&addr, s);
}