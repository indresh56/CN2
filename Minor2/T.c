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

#include <netinet/tcp.h>
int check(int store[],int port,int size)
{
    for(int i=0;i<size;i++){
        if(store[i]==port) return 0;
    }
    return 1;
}
int main()
{
    int store[100];
    char storeip[100][20];
    // int storeclientport[100];

    int size=0;
    int rsfd=socket(AF_INET,SOCK_RAW,IPPROTO_TCP);
    int sfd = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
    if (sfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

   
    struct sockaddr_in serveaddr;
    serveaddr.sin_family = AF_INET;
    serveaddr.sin_port = htons(9090); 
    serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    
    if (connect(sfd, (struct sockaddr *)&serveaddr, sizeof(serveaddr)) < 0) {
        perror("Connection to server failed");
        close(sfd);
        exit(EXIT_FAILURE);
    }
     int rsfd2 = socket(AF_INET, SOCK_RAW, 150);
    if(rsfd2<0)
        perror("raw socket fail: ");

    while(1)
    {
        struct sockaddr_in saddr;
        socklen_t s=sizeof(saddr);
        char buffer[65536];
        int a=recvfrom(rsfd,buffer,65536,0,(struct sockaddr*)&saddr,&s);
        if(a>0)
        {
            struct iphdr *ip_header = (struct iphdr *)(buffer);
            struct tcphdr *tcp_header = (struct tcphdr *)(buffer + ip_header->ihl * 4);
            if (tcp_header->syn == 1 && tcp_header->ack == 0&&ntohs(tcp_header->dest)!=9090) {
                int serverport=ntohs(tcp_header->dest);
                printf("Port, destination: %d source: %d\n",serverport,ntohs(tcp_header->source));
                printf("IP, destination: %s source: %s\n",inet_ntoa(*(struct in_addr *)&ip_header->daddr),inet_ntoa(*(struct in_addr *)&ip_header->saddr));
               fflush(stdout);
               if(check(store,serverport,size)==1)
                {
                    char b[256];
                    snprintf(b,sizeof(b),"%d",serverport);
                    printf("%s\n",b);
                    if(send(sfd,b,sizeof(b),0)<0){
                        perror("send failed: ");
                    }
                    store[size]=serverport;
                    strcpy(storeip[size],inet_ntoa(*(struct in_addr *)&ip_header->daddr));
                    size++;
                    //store source ip and dest
                    for(int i=0;i<size-1;i++)
                    {
                            struct sockaddr_in dest;
                            dest.sin_family = AF_INET;
                            dest.sin_addr.s_addr = inet_addr(storeip[i]);
                            if (sendto(rsfd2, b, 256, 0, (struct sockaddr *) &dest, sizeof(dest)) < 0) {
                                perror("sendto failed");
                            }
                    }
                }
            }
        }
    }

}