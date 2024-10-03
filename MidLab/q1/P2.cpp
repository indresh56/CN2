#include<time.h>
#include<bits/stdc++.h>
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<sys/select.h>
#include<pthread.h>
#include<signal.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/shm.h>
#include<unistd.h>
#include<sys/un.h>
#include<netinet/ip.h>
#include<arpa/inet.h>
#include<errno.h>
#include<netinet/if_ether.h>
#include<net/ethernet.h>
#include<netinet/ether.h>
#include<netinet/udp.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include <sys/sem.h>
#include<poll.h>
using namespace std;
int main()
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
    if (sfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
// int opt = 1;
//        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
//         perror("setsockopt failed");
//         close(sfd);
//         exit(EXIT_FAILURE);
//     }
//     if (setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
//         perror("setsockopt failed");
//         close(sfd);
//         exit(EXIT_FAILURE);
//     }
    // Define server address
    struct sockaddr_in serveaddr;
    serveaddr.sin_family = AF_INET;
    serveaddr.sin_port = htons(7070); // change port no for different processes
    serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(sfd, (struct sockaddr *)&serveaddr, sizeof(serveaddr)) < 0) {
        perror("Connection to server failed");
        close(sfd);
        exit(EXIT_FAILURE);
    }
    while(1)
    {
    int n;
    cin>>n;
    char str[20];
    sprintf(str, "%d", n);
    if (send(sfd, str, strlen(str), 0) < 0) {
        perror("Send failed");
        close(sfd);
        exit(EXIT_FAILURE);
    }
    }
    

}