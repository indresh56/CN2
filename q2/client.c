#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
int main()
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
    struct sockaddr_in serveaddr;
    serveaddr.sin_family = AF_INET;
    serveaddr.sin_port = htons(8080);//change port no according to service
    serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connect(sfd, (struct sockaddr *)&serveaddr, sizeof(serveaddr));
    /* char *str = "Hello Server";
     send(sfd, str, strlen(str), 0);*/

     
     /*
     for connection less
     char buff[100];
      int n = recvfrom(sfd, buff, 100, 0, (struct sockaddr *)&serveraddress, &addr_len);
    buff[n] = '\0';
    printf("Received from server: %s", buff);
      */
    char buff[100];
    int n = recv(sfd, buff, sizeof(buff), 0);
    buff[n] = '\0';

    printf("Received from Server: %s\n", buff);
}