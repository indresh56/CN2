#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<poll.h>
#include<pthread.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
sem_t mutex;
struct sockaddr_in server_addr,client_addr;
socklen_t socklen_server,socklen_client;
pthread_t tid1,tid2;
int ffd;
int sfd_news,nsfd;
char buffer[1024];
char temp_buffer[5];
int which=1;
int port_num;
int live=0;
void* reader_1(void* args)
{
    while(1)
    {
        while(which==0);
        sem_wait(&mutex);
        // recv(nsfd,temp_buffer,sizeof(temp_buffer),0);
        // if(temp_buffer[0]>=48 && temp_buffer[0]<=57)
        // {
        //     port_num=atoi(temp_buffer);
        // }
        //connect socket
        //while(1)recv()
        recv(nsfd,buffer,sizeof(buffer),0);
        printf("News reader-1:%s\n",buffer);
        which=0;
        sem_post(&mutex);
    }
}
void* reader_2(void* args)
{
    while(1)
    {
        while(which==1);
        sem_wait(&mutex);
        // recv(nsfd,temp_buffer,sizeof(temp_buffer),0);
        // if(temp_buffer[0]>=48 && temp_buffer[0]<=57)
        // {
        //     port_num=atoi(temp_buffer);
        // }
        //connect socket
        //while(1)recv()
        recv(nsfd,buffer,sizeof(buffer),0);
        printf("News reader-2:%s\n",buffer);
        which=1;
        sem_post(&mutex);
    }
}
int main()
{
    sfd_news=socket(AF_INET,SOCK_STREAM,0);
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=INADDR_ANY;
	server_addr.sin_port=htons(10000);
	socklen_server=sizeof(server_addr);
	if(bind(sfd_news,(struct sockaddr*)&server_addr,socklen_server)<0)
	{
		printf("Error in binding");
		return 0;
	}
    listen(sfd_news,5);
    nsfd=accept(sfd_news,(struct sockaddr*)&client_addr,&socklen_client);
    sem_init(&mutex,0,1);
    pthread_create(&tid1,NULL,&reader_1,NULL);
    pthread_create(&tid2,NULL,&reader_2,NULL);
    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);
}