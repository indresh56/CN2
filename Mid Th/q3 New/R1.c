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
    serveaddr.sin_port = htons(7070); // change port no for different processes
    serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(connect(sfd, (struct sockaddr *)&serveaddr, sizeof(serveaddr))==-1)
    {
        perror("Error in connection");
    }
    printf("Connection Done\n");
     char s[200];
    fgets(s, sizeof(s), stdin);
    send(sfd,s,sizeof(s),0);
    
}

/*int fd = open("Rep1", O_WRONLY);
    char s[200];
    fgets(s, sizeof(s), stdin);
    // cout << s << endl;
    write(fd, s, strlen(s));
    close(fd); */