#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <signal.h>
struct msg
{
    long type;
    struct sockaddr_in addr;
};
int main()
{
    // Service first client
    //  char buff[200];
    //  scanf(" %s",buff);
    printf("First time serviced by S1");
    // listen for other client
    int l = fork();
    if (l == 0)
    {
        key_t key = ftok("msgq", 65);
       int msqid = msgget(key, 0666 | IPC_CREAT);
      
        int sfd = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
        int reuse = 1;
        struct sockaddr_in serveaddr, clientaddr;
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
        serveaddr.sin_family = AF_INET;
        serveaddr.sin_port = htons(8080);
        serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        bind(sfd, (struct sockaddr *)&serveaddr, sizeof(serveaddr));
        listen(sfd, 5); // 5 is the wait no
        socklen_t clientsize;
        while (1)
        {
            int nsfd = accept(sfd, (struct sockaddr *)&clientaddr, &clientsize);
             struct msg M;
            M.type=1;
            M.addr=clientaddr;
            msgsnd(msqid,&M,sizeof(M.addr),0);
            char buffer[128];
            int pid;
            FILE *fp= popen("pgrep server", "r");
            fgets(buffer, sizeof(buffer), fp);
            pid = atoi(buffer); 
          
            kill(pid,SIGUSR1);
           
            int c = fork();
            if (c > 0)
                close(nsfd);
            else
            {
                close(sfd);
                //  printf("Client Connected\n");
                //  char *buff[100]; //="Hello World";

                // recv(nsfd, buff, sizeof(buff), 0);
                // printf("Received from client: %s\n", buff);
                char *str = "Hello Client from S1";
                send(nsfd, str, strlen(str), 0);
                close(nsfd);
                return 0;
            }
        }
    }
}