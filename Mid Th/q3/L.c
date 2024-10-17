#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

int main()
{
    int sfd;
        sfd = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
        struct sockaddr_in serveaddr,clientaddr;

        serveaddr.sin_family = AF_INET;
        serveaddr.sin_port = htons(8080);
        serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        bind(sfd, (struct sockaddr *)&serveaddr, sizeof(serveaddr));
        listen(sfd, 5); // 5 is the wait no
        while(1)
        {
            
        socklen_t clientsize=sizeof(clientaddr);
        
        int nsfd = accept(sfd, (struct sockaddr *)&clientaddr, &clientsize);
        int c=fork();
        if(c>0) close(nsfd);
        else{
            close(sfd);
        // char s[200];
        // fgets(s, sizeof(s), stdin);
        // printf("%s",s);
        // fflush(stdout);
        char *s="Live telecaster news...............\n";
        // cout << s << endl;
        send(nsfd, s, strlen(s),0);
        close(nsfd);
        return 0;
        }
        }
   
   

}