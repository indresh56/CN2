#include<stdio.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<poll.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
int main()
{
    int sfd=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in server_addr;
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	server_addr.sin_port=htons(9000);
	socklen_t socklen=sizeof(server_addr);
	connect(sfd,(struct sockaddr*)&server_addr,socklen);
	char buffer[1024]="/dSending message to Server from Reporter-1\n";
	int s=send(sfd,buffer,sizeof(buffer),0);
	if(s<0)
	{
		printf("Error in sending");
	}
	else if(s==0)
	{
		printf("Connection Closed at sending\n");
	}
	close(sfd);
}