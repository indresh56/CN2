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
int msqid2,msqid3;
int rd=0;

void handle(int sig)
{
 if(rd==0) rd=1;
 else rd=0;
}
void handle2(int sig)
{
    struct msg M;
    M.type=1;
    strcpy(M.message,"exit");
  msgsnd(msqid3,&M,sizeof(M.message),0);
  exit(0);
}

int main()
{
    signal(SIGINT,handle2);
    signal(SIGUSR1,handle);
    key_t k;

    k=ftok("Message",65);
     msqid2=msgget(k,0666|IPC_CREAT);

    k=ftok("Profs",65);
       msqid3=msgget(k,0666|IPC_CREAT);  
    struct msg M;
    msgrcv(msqid2,&M,sizeof(M.message),1,0); 

     char buffer[128];
    int pid;
    FILE *fp= popen("pgrep P2", "r");
    fgets(buffer, sizeof(buffer), fp);
    pid = atoi(buffer); 
    while(1)
    {
        if(rd==1)
        {
            struct msg Temp;
            msgrcv(msqid2,&Temp,sizeof(Temp.message),0,0);
            printf("%s",Temp.message);
            if(Temp.message[0]>='0'&&Temp.message[0]<='9'&&Temp.message[1]>='0'&&Temp.message[1]<='9'&&Temp.message[2]>='0'
            &&Temp.message[2]<='9'&&Temp.message[3]>='0'&&Temp.message[3]<='9')
            {
                int sfd = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
                struct sockaddr_in serveaddr;
                char port[4];
                for(int i=0;i<4;i++) port[i]=M.message[i];

                serveaddr.sin_family = AF_INET;
                serveaddr.sin_port = atoi(port);
                serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

                connect(sfd, (struct sockaddr *)&serveaddr, sizeof(serveaddr));
                char *buff="Professor Contacted Personally";
                send(sfd,buff,strlen(buff),0);
                close(sfd);
            }
            kill(pid,SIGUSR1);
        }
        else{
            struct msg Temp;
            Temp.type=1;
            strcpy(Temp.message,"Written by P1");
            msgsnd(msqid3,&Temp,sizeof(Temp.message),0);
            kill(pid,SIGUSR1);
        }
    }
}