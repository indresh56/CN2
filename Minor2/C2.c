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

int main()
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
    if (sfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serveaddr;
    serveaddr.sin_family = AF_INET;
    serveaddr.sin_port = htons(5051); 
    serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  
    if (connect(sfd, (struct sockaddr *)&serveaddr, sizeof(serveaddr)) < 0) {
        perror("Connection to server failed");
        close(sfd);
        exit(EXIT_FAILURE);
    }
    printf("Connected\n");
    char buff[256];
    if(recv(sfd,buff,sizeof(buff),0)<0){
        perror("receive: ");
    }
    printf("%s\n",buff);
    fflush(stdout);
     int rsfd = socket(AF_INET, SOCK_RAW, 150);
    if(rsfd<0)
        perror("raw socket fail: ");
    while(1)
    {
        char buff[256];
        struct sockaddr_in addr;
        int siz=sizeof(addr);
        if(recvfrom(rsfd,buff,sizeof(buff),0,(struct sockaddr*)&addr,&siz)<0) continue;
        else printf("New Service: %s\n",buff+((struct iphdr*)buff)->ihl*4);
    }


}