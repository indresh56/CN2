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
int livetelecast=0;
void handle1(int sig)
{
    if(livetelecast==0) livetelecast=1;
    else livetelecast=0;
}
struct msg
{
     long type;
    char message[200];
   
};
int main()
{
 signal(SIGUSR1,handle1);
 key_t k=ftok("news",65);
 int msqid=msgget(k,0666|IPC_CREAT);
 printf("%d",msqid);
 while(1)
 {
    while(livetelecast==1);
    struct msg M;
    if (msgrcv(msqid, &M, sizeof(M.message), 1, 0) == -1) {
            perror("msgrcv failed");
            continue; 
        }
    // msgrcv(msqid,&M,sizeof(M.message),1,0);
    // printf("%s",M.message);
    // fflush(stdout);
    if(M.message[0]>='0'&&M.message[0]<='9'&&M.message[1]>='0'
    &&M.message[1]<='9'&&M.message[2]>='0'&&M.message[2]<='9'
    &&M.message[3]>='0'&&M.message[3]<='9')
    {
    
    int sfd = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
    struct sockaddr_in serveaddr;
    char port[4];
    for(int i=0;i<4;i++) port[i]=M.message[i];

    serveaddr.sin_family = AF_INET;
    serveaddr.sin_port = htons(8080);//stoi(port)
    serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sfd, (struct sockaddr *)&serveaddr, sizeof(serveaddr));
    printf("Connected to L");
    // fflush(stdout);
    char buffer[128];
    int pid;
    FILE *fp= popen("pgrep N2", "r");
    fgets(buffer, sizeof(buffer), fp);
    pid = atoi(buffer); 
    // printf("%d",pid);
    // fflush(stdout);
    kill(pid,SIGUSR1);
    livetelecast=1;
     char buff[255];
    int n = recv(sfd, buff, sizeof(buff), 0);
    printf("%d",n);
       fflush(stdout);
    buff[n] = '\0';
    printf("%s",buff);
    fflush(stdout);
    struct msg Temp;
    Temp.type=3;
    strcpy(Temp.message,buff);
    //Temp.message=buff;
    msgsnd(msqid,&Temp,sizeof(Temp.message),0);
    livetelecast=0;
    // fp= popen("pgrep N2", "r");
    // fgets(buffer, sizeof(buff), fp);
    // pid = atoi(buffer); 
    kill(pid,SIGUSR1);
    close(sfd);
    }
    else{
         struct msg Temp;
    Temp.type=3;
    strcpy(Temp.message,M.message);
    //Temp.message=M.message;
    msgsnd(msqid,&Temp,sizeof(Temp.message),0);
    }

 }
}