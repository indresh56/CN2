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

int main()
{
    int sfd[3];
     //mkfifo("Rep", 0666);
    //  if(mkfifo("Rep",0666)==-1)
	// {
	// 	perror("mkfifo()1");
	// 	exit(1);
	// }
    int fd;
     if( (fd=open("Rep", O_RDWR))==-1)
     {
        perror("open()");
		exit(1);
     }
    for (int i = 0; i < 3; i++)
    {
        sfd[i] = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
        struct sockaddr_in serveaddr;
        serveaddr.sin_family = AF_INET;
        if (i == 0)
            serveaddr.sin_port = htons(7070);
        else if (i == 1)
            serveaddr.sin_port = htons(7071);
        else
            serveaddr.sin_port = htons(7072);
        serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        bind(sfd[i], (struct sockaddr *)&serveaddr, sizeof(serveaddr));
        listen(sfd[i], 5); // 5 is the wait no
    }
    struct pollfd P[3];
    for (int i = 0; i < 3; i++)
    {
        P[i].fd = sfd[i];
        P[i].events = POLLIN;
    }
    while(1)
    {
        int a=poll(P,3,100);
        if(a>0)
        {
            for (int i = 0; i < 3; i++)
            {
                if (P[i].revents & POLLIN)
                {
                    struct sockaddr_in clientaddr;
                    int clientsize=sizeof(clientaddr);
                    
                    int nsfd=0;
                  
                    if((nsfd = accept(P[i].fd, (struct sockaddr *)&clientaddr, &clientsize))==-1)
                    {
                        perror("Problem in accepting");
                    }
                    //  printf("%d ",nsfd);
                    // fflush(stdout);
                    
                    
                   
                    while(1)
                    {
                        char s[200];
                     ssize_t bytesRead = recv(nsfd, s, sizeof(s),0);
                    if (bytesRead <= 0)
                    {
                        continue;
                    }
                    else
                    {
                    s[bytesRead]='\0';
                    printf("%s",s);
                    fflush(stdout);
                    write(fd,s,sizeof(s));
                    break;
                    }
                    }
                    close(nsfd);
                }
            }
        }
    }
    
}
//int t = recv(sfd, buff, sizeof(buff), 0);
//    buff[t] = '\0';