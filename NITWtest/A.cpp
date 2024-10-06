#include<time.h>
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
#include<pcap.h>

int noOfques=3;
int main()
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
    if (sfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serveaddr;
    serveaddr.sin_family = AF_INET;
    serveaddr.sin_port = htons(8080); // change port no for different processes
    serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");


    if (connect(sfd, (struct sockaddr *)&serveaddr, sizeof(serveaddr)) < 0) {
        perror("Connection to server failed");
        close(sfd);
        exit(EXIT_FAILURE);
    }
    char quespaper[100];
    recv(sfd,quespaper,sizeof(quespaper),0);
    printf("%s",quespaper);
    fflush(stdout);
    char *str = "8080";
    if (send(sfd, str, strlen(str), 0) < 0) {
        perror("Send failed");
        close(sfd);
        exit(EXIT_FAILURE);
    }
    // if(mkfifo("fifo",0666)==-1)
    // {
    //     perror("fifo creation");
    // }
    int ffd=open("fifo",O_RDONLY);
    struct pollfd PFD[2];
    PFD[0].fd=0;
    PFD[1].fd=ffd;
    PFD[0].events=POLLIN;
    PFD[1].events=POLLIN;
    
    int countOfques=0;
    while(1)
    {
        int a=poll(PFD,2,-1);
        if(a>0)
        {
            for(int i=0;i<2;i++)
            {
            if(PFD[i].revents&POLLIN)
                {
                    if(i==0)
                    {
                        char buff[100];
                        int t=read(PFD[i].fd,buff,sizeof(buff));
                        buff[t]='\0';
                        if(send(sfd,buff,sizeof(buff),0)<0)
                        {
                            perror("Send failed");
                            close(sfd);
                            exit(EXIT_FAILURE);
                        }
                        countOfques++;

                    }
                    else
                    {
                        char buff[100];
                         int t=read(PFD[i].fd,buff,sizeof(buff));
                        buff[t]='/0';
                        printf("Warning:%s",buff);
                        
                    }
                }
            }
        }
        if(countOfques==noOfques) break;
    }
    char buff[100];
    int size=recv(sfd,buff,sizeof(buff),0);
    buff[size]='\0';
    printf("%s",buff);
    fflush(stdout);
    close(sfd);


}