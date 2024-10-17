#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include<signal.h>
#include <sys/msg.h>
#include <stdbool.h>
struct msg{
    long type;
    char message[200];
};
int main()
{

    struct msg M;
    M.type=getpid();
    strcpy(M.message,"Joining");
    key_t k=ftok("Join",65);
    int msqid=msgget(k,0666|IPC_CREAT);
    msgsnd(msqid,&M,sizeof(M.message),0);


    k=ftok("Message",65);
    int msqid2=msgget(k,0666|IPC_CREAT);

  
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serveraddress,clientaddr;
    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(8080);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(sfd, (struct sockaddr *)&serveraddress, sizeof(serveraddress));
     struct pollfd PFD[4];
     PFD[0].fd=msqid2;
     PFD[1].fd=sfd;
     PFD[2].fd=stdin;
     PFD[3].fd=msqid;
     for(int i=0;i<4;i++) PFD[i].events=POLLIN;
     while(1)
     {
        int a=poll(PFD,4,100);
        if(a>0)
        {
            for(int i=0;i<4;i++)
            {
                if(PFD[i].fd&POLLIN)
                {
                    if(i==0)
                    {
                        char buff[200];
                        struct msg T;
                        if(msgrcv(PFD[i].fd,&T,sizeof(T.message),getpid(),0)==-1)
                        {
                            perror("msgrcv failed");
                            continue; 
                        }
                        printf("%s",T.message);
                    }
                    else if(i==1)
                    {
                        socklen_t clientsize=sizeof(clientaddr);
                         int nsfd = accept(sfd, (struct sockaddr *)&clientaddr, &clientsize);
                         char buff[200];
                         int bytes_received = recv(nsfd, buff, sizeof(buff) - 1, 0);
                        if (bytes_received > 0) {
                            buff[bytes_received] = '\0';  // Null-terminate the string
                            printf("Received from professor: %s\n", buff);
                        } else if (bytes_received == 0) {
                            // Connection closed by peer
                            printf("Professor disconnected.\n");
                        } else {
                            continue;
                        }
                         //recv(nsfd,buff,200,0);
                         printf("Received from professor %s",buff);
                         close(nsfd);
                    }
                    else if(i==2){
                        char buff[200];
                        int a=read(0, buff, 200);
                        if(a!=0)
                        {
                        buff[a]='\0';
                        struct msg T;
                        T.type=getpid();
                        strcpy(T.message,buff);
                        msgsnd(msqid2,&T,sizeof(T.message),0);
                        }
                    }
                    else{
                        //for attendance 
                        msgrcv(msqid,&M,sizeof(M.message),getpid(),0);
                        printf("%s",M.message);
                        return 0;
                    }
                }
            }
        }
     }
    // socklen_t addr_len = sizeof(serveraddress);
    // char buff[100];
    // char *msg = "Student1";
    // sendto(sfd, msg, strlen(msg), 0, (struct sockaddr *)&serveraddress, sizeof(serveraddress));

    // int n = recvfrom(sfd, buff, 100, 0, (struct sockaddr *)&serveraddress, &addr_len);
    // buff[n] = '\0';
    // printf("Received from server: %s", buff);
}