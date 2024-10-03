

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
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1) {
        perror("Socket creation failed");
        close(sfd);
        return 1;
    }
   
    struct sockaddr_in serveraddress, clientaddress;
    socklen_t addr_len = sizeof(clientaddress);
    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(7075);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sfd, (struct sockaddr *)&serveraddress, sizeof(serveraddress)) == -1) {
        perror("Bind failed");
        close(sfd);
        return 1;
    }
    struct pollfd P[1];
    P[0].fd=sfd;
    P[0].events=POLLIN;
    while(1)
    {
        int a=poll(P,1,100);
        if(a>0&&P[0].revents&POLLIN)
        {
            char buff[100];

     
        int n = recvfrom(sfd, buff, sizeof(buff) - 1, 0, (struct sockaddr *)&clientaddress, &addr_len);
        if (n == -1) {
            perror("Recvfrom failed");
            close(sfd);
            return 1;
        }


        buff[n] = '\0';
        printf("%s\n", buff);

        }
    }
}
/*



 */
    // int sfd1,sfd2,sfd3;
    // sfd1 = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
    //     if (sfd1 < 0) {
    //         perror("Socket creation failed");
    //         exit(EXIT_FAILURE);
    //     }
    // sfd2 = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
    // if (sfd2 < 0) {
    //     perror("Socket creation failed");
    //     exit(EXIT_FAILURE);
    // }
    // sfd3 = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
    // if (sfd3 < 0) {
    //     perror("Socket creation failed");
    //     exit(EXIT_FAILURE);
    // }
    //      struct sockaddr_in serveaddr1;
    //     serveaddr1.sin_family = AF_INET;
    //     serveaddr1.sin_port = htons(7075);
    //     serveaddr1.sin_addr.s_addr = inet_addr("127.0.0.1");
    //     if (bind(sfd1, (struct sockaddr *)&serveaddr1, sizeof(serveaddr1)) < 0) {
    //         perror("Bind failed");
    //         close(sfd1);
    //         exit(EXIT_FAILURE);
    //     }
    //      if (listen(sfd1, 5) < 0) { // 5 is the backlog
    //         perror("Listen failed");
    //         close(sfd1);
    //         exit(EXIT_FAILURE);
    //     }
    //     struct sockaddr_in serveaddr2;
    //     serveaddr2.sin_family = AF_INET;
    //     serveaddr2.sin_port = htons(7076);
    //     serveaddr2.sin_addr.s_addr = inet_addr("127.0.0.1");
    //     if (bind(sfd2, (struct sockaddr *)&serveaddr2, sizeof(serveaddr2)) < 0) {
    //         perror("Bind failed");
    //         close(sfd2);
    //         exit(EXIT_FAILURE);
    //     }
    //      if (listen(sfd2, 5) < 0) { // 5 is the backlog
    //         perror("Listen failed");
    //         close(sfd2);
    //         exit(EXIT_FAILURE);
    //     }
    //     struct sockaddr_in serveaddr3;
    //     serveaddr3.sin_family = AF_INET;
    //     serveaddr3.sin_port = htons(7077);
    //     serveaddr3.sin_addr.s_addr = inet_addr("127.0.0.1");
    //     if (bind(sfd3, (struct sockaddr *)&serveaddr3, sizeof(serveaddr3)) < 0) {
    //         perror("Bind failed");
    //         close(sfd3);
    //         exit(EXIT_FAILURE);
    //     }
    //      if (listen(sfd3, 5) < 0) { // 5 is the backlog
    //         perror("Listen failed");
    //         close(sfd3);
    //         exit(EXIT_FAILURE);
    //     }

       
 
    //     struct pollfd P[3];
    //     P[0].fd=sfd1;
    //     P[1].fd=sfd2;
    //     P[2].fd=sfd3;
    //     P[0].events=POLLIN;
    //     P[1].events=POLLIN;
    //     P[2].events=POLLIN;
    //     for(int i=0;i<3;i++)
    //     {
    //         struct sockaddr_in clientaddr;
    //         socklen_t clientsize = sizeof(clientaddr);
    //         int nsfd = accept(P[i].fd, (struct sockaddr *)&clientaddr, &clientsize);
    //         if (nsfd < 0) {
    //             perror("Accept failed");
    //             continue;
    //         }
    //         P[i].fd=nsfd;
    //     }
    //     while(1)
    //     {
    //         int a=poll(P,3,100);
    //         if(a>0)
    //         {
    //             for(int i=0;i<3;i++)
    //             {

    //                 if(P[i].revents&POLLIN)
    //                 {
    //                     // struct sockaddr_in clientaddr;
    //                     // socklen_t clientsize = sizeof(clientaddr);
    //                     // int nsfd = accept(P[i].fd, (struct sockaddr *)&clientaddr, &clientsize);
    //                     // if (nsfd < 0) {
    //                     //     perror("Accept failed");
    //                     //     continue;
    //                     // }
    //                     char buff[100];
    //                     recv(P[i].fd,buff,sizeof(buff),0);
    //                     printf("%s\n",buff);
    //                     fflush(stdout);
                      
                        
    //                 }
    //             }
    //         }
    //     }

// }
    
 