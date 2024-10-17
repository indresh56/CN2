#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <sys/ipc.h>
#include <sys/msg.h>
struct msg
{
     long type;
    char message[200];
   
};
int main()
{
    mkfifo("Rep1", 0666);
    mkfifo("Rep2", 0666);
    mkfifo("Rep3", 0666);
    mkfifo("DocWr",0666);
    // key_t key=ftok("news",65);
    key_t key = ftok("news", 65);
    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }
    int msqid=msgget(key,0666|IPC_CREAT);
    //printf("%d",msqid);
    int rfd[3];
    rfd[0] = open("Rep1", O_RDWR);
    rfd[1] = open("Rep2", O_RDWR);
    rfd[2] = open("Rep3", O_RDWR);
    int dfd=open("DocWr",O_RDWR);
    struct pollfd prfd[3];
    for (int i = 0; i < 3; i++)
    {
        prfd[i].fd = rfd[i];
        prfd[i].events = POLLIN;
    }
    int whichnewsreader=1;
    while (1)
    {
        int a = poll(prfd, 3, 100);
        if (a > 0)
        {
            for (int i = 0; i < 3; i++)
            {
                if (prfd[i].revents & POLLIN)
                {
                    char s[200];
                    ssize_t bytesRead = read(prfd[i].fd, s, 200);
                   
                    if (bytesRead <= 0)
                    {
                        continue;
                    }
                    // read(prfd[i].fd, s, sizeof(s));
                    else
                    {
                        s[bytesRead]='\0';
                        if(s[0]=='/'&&s[1]=='d')
                        {
                            char str[198];
                            for(int i=0;i<198;i++) str[i]=s[i+2];
                            write(dfd,str,bytesRead);
                        }
                        else{
                             
                           
                                
                                struct msg M;
                                M.type=whichnewsreader;
                                 strncpy(M.message, s, sizeof(M.message) - 1);
                                M.message[sizeof(M.message) - 1] = '\0';
                                // strcpy(M.message,s);
                                // M.message[strlen(s)] = '\0'; 
                                printf("%s",M.message);
                                fflush(stdout);
                                if (msgsnd(msqid, &M, sizeof(M.message), 0) == -1) {
                                    perror("msgsnd failed");}
                                //msgsnd(msqid,&M,sizeof(M.message),0);
                                if(whichnewsreader==1)
                                whichnewsreader=2;
                            else whichnewsreader=1;

                           
                                fflush(stdout);

                            
                            
                            
                        }
                        //printf("%s\n", s);
                    }
                }
            }
        }
    }
}