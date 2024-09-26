#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>


int main()
{
     int wfd = open("DocWr", O_RDWR);
     int ffd=open("docwrite.txt",O_WRONLY | O_CREAT | O_APPEND, 0644);
     struct pollfd PFD[1];
     PFD[0].fd=wfd;
     PFD[0].events=POLLIN;
     while(1)
     {
        int a=poll(PFD,1,100);
        if(a>0)
        {
            char s[198];
            int t=read(wfd,s,sizeof(s));
            s[t]='\0';
            printf("%d",t);
            write(ffd,s,t);
        }
     }

}