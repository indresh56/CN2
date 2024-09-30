#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

int main()
{
    int sfd[4];
    for (int i = 0; i < 2; i++)
    {
        sfd[i] = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
        struct sockaddr_in serveaddr;
        serveaddr.sin_family = AF_INET;
            serveaddr.sin_port = htons(8080+i);
        
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
    while (1)
    {
        int t = poll(P, 3, 100);
        if (t > 0)
        {
            for (int i = 0; i < 3; i++)
            {
                if (P[i].revents & POLLIN)
                {
                    struct sockaddr_in clientaddr;
                    int clientsize;
                    int nsfd = accept(sfd[i], (struct sockaddr *)&clientaddr, &clientsize);
                    int c = fork();
                    if (c > 0)
                        close(nsfd);
                    else
                    {
                        close(sfd[i]);
                        dup2(nsfd, 0);
                        dup2(nsfd, 1);
                        if (i == 0)
                            execl("./A", "A", NULL);
                        else if (i == 1)
                            execl("./B", "B", NULL);
                        else
                            execl("./C", "C", NULL);

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
        }
    }
}