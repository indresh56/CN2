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
#include<stdbool.h>
struct msg{
    long type;
    char message[200];
};

int main()
{
    // int sfd[3];
    // struct sockaddr_in serveraddress, clientaddress;
    // struct studentaddr
    // for(int i=0;i<3;i++)
    // {
    //     sfd[i] = socket(AF_INET, SOCK_DGRAM, 0);
    // socklen_t addr_len = sizeof(clientaddress);
    // serveraddress.sin_family = AF_INET;
    // serveraddress.sin_port = htons(8080+i);
    // serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    // bind(sfd, (struct sockaddr *)&serveraddress, sizeof(serveraddress));
    // }
    // struct pollfd PFD[4];
    // for(int i=0;i<3;i++) 
    // {
    //     PFD[i].fd=sfd[i];
    // }

  
    key_t k=ftok("Join",65);
    int msqid=msgget(k,0666|IPC_CREAT);

    k=ftok("Message",65);
    int msqid2=msgget(k,0666|IPC_CREAT);

    k=ftok("Profs",65);
      int msqid3=msgget(k,0666|IPC_CREAT);
    bool studentjoined=false;
    int studPID[20];
    int studMsgOnce[20];
    int n=0;
    for(int i=0;i<20;i++) studMsgOnce[i]=0;
    struct pollfd PFD[3];
    PFD[0].fd=msqid;
    PFD[1].fd=msqid2;
    PFD[2].fd=msqid3;
    for(int i=0;i<3;i++) PFD[i].events=POLLIN;

    int prof1=false,prof2=false;
    while(1)
    {
        for(int i=0;i<3;i++)
        {
            if(PFD[i].revents&POLLIN)
            {
                struct msg M;
                if(i==0)
                {
                    msgrcv(PFD[i].fd,&M,sizeof(M.message),0,0);
                    studPID[n]=M.type;
                    if(n==0)
                    {
                      struct msg Temp;
                      Temp.type=1;
                      msgsnd(msqid3,&Temp,sizeof(Temp.message),0);
                      Temp.type=2;
                      msgsnd(msqid3,&Temp,sizeof(Temp.message),0);
                    }
                    n++;
                }
                else if(i==1)
                {
                    msgrcv(PFD[i].fd,&M,sizeof(M.message),0,0);
                    for(int i=0;i<n;i++)
                    {
                        if(studPID[i]==M.type) studMsgOnce[i]=1;
                    }
                    msgsnd(msqid3,&M,sizeof(M.message),0);
                }
                else{
                    msgrcv(PFD[i].fd,&M,sizeof(M.message),0,0);
                    if(M.type==1&&strcmp("exit",M.message)==0) prof1=true;
                    else if(M.type==2&&strcmp("exit",M.message)==0) prof2=true;
                    else{
                        for(int i=0;i<n;i++)
                        {
                            M.type=studPID[i];
                            msgsnd(msqid2,&M,sizeof(M.message),0);
                        }
                    }
                }
            }
        }
        if(prof1&&prof2) break;

    }
    for(int i=0;i<n;i++)
    {
        struct msg M;
        M.type=studPID[i];
        if(studMsgOnce[i]==1)
        strcpy(M.message,"Attendance taken and praising certificate\n");
        else strcpy(M.message,"Attendance taken\n");

        msgsnd(msqid,&M,sizeof(M.message),0);
    }
    //one for requests one for msg  two fds for profs
    // char buff[100];
    // int n = recvfrom(sfd, buff, 100, 0, (struct sockaddr *)&clientaddress, &addr_len);
    // buff[n] = '\0';
    // printf("Received from client: %s", buff);
    // char *msg = "Sent from server";
    // sendto(sfd, msg, strlen(msg), 0, (struct sockaddr *)&clientaddress, addr_len);
}

//poll stdin in 