#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <signal.h>
struct msg
{
    long type;
    struct sockaddr_in addr;
};

int msqid;
struct sockaddr_in LastTen[10];
int N=0;
void handle(int sig)
{
    struct msg M;
    msgrcv(msqid, &M, sizeof(M.addr), 1, 0);
    LastTen[N]=M.addr;
    N=(N+1)%10;
}

int main()
{
    int sfd[5];
      key_t key = ftok("msgq", 65);
     msqid = msgget(key, 0666 | IPC_CREAT);
     signal(SIGUSR1,handle);
    for (int i = 0; i < 5; i++)
    {
        sfd[i] = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
        int reuse = 1;
        setsockopt(sfd[i], SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        setsockopt(sfd[i], SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
        struct sockaddr_in serveaddr;
        serveaddr.sin_family = AF_INET;
        serveaddr.sin_port = htons(8080 + i);
        serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        bind(sfd[i], (struct sockaddr *)&serveaddr, sizeof(serveaddr));
        listen(sfd[i], 5); // 5 is the wait no
    }
    struct pollfd P[5];
    for (int i = 0; i < 5; i++)
    {
        P[i].fd = sfd[i];
        P[i].events = POLLIN;
    }
    struct sockaddr_in A[10];
    int n = 0;
    int check[5] = {0, 0, 0, 0, 0};
    while (1)
    {
        int t = poll(P, 5, 100);
        if (t > 0)
        {

            for (int i = 0; i < 5; i++)
            {
                // printf("%d", i);
                if (P[i].revents & POLLIN)
                {

                    if (i == 0 || i == 2 || i == 4)
                    {
                        if (check[i] == 0)
                        {
                            check[i]++;
                            struct sockaddr_in clientaddr;
                            socklen_t clientsize;
                            int nsfd = accept(sfd[i], (struct sockaddr *)&clientaddr, &clientsize);
                            LastTen[N]=clientaddr;
                            N=(N+1)%10;
                           
                            int c = fork();
                            if (c > 0)
                            {
                                close(nsfd);
                                close(sfd[i]);
                            }
                            else
                            {
                                close(sfd[i]);
                                dup2(nsfd, 0);
                                dup2(nsfd, 1);
                                if (i == 0)
                                    execl("./S1", "S1", NULL);
                                else if (i == 2)
                                    execl("./S3", "S2", NULL);
                                else
                                    execl("./S5", "S5", NULL);

                                // printf("Client Connected\n");
                                // char *buff[100]; //="Hello World";
                                // recv(nsfd, buff, sizeof(buff), 0);
                                // printf("Received from client: %s\n", buff);
                                // char *str = "Hello Client";
                                // send(nsfd, str, strlen(str), 0);
                                return 0;
                            }
                        }
                    }
                    else
                    {
                        printf("Servicing connection less\n");
                        if(i==1)
                        {
                         char buff[100];
                        char *msg = "Sent from Server connection less for S2";
                        sendto(sfd[i], msg, strlen(msg), 0, (struct sockaddr *)&clientaddress, addr_len);
                        }
                        else if(i==3)
                        {
                             char buff[100];
                        char *msg = "Sent from Server connection less for S4";
                        sendto(sfd[i], msg, strlen(msg), 0, (struct sockaddr *)&clientaddress, addr_len);
                        }
                        // servicing connectionless
                    }
                }
            }
        }
    }
}