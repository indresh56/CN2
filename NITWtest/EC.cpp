#include<time.h>
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<sys/select.h>
#include<pthread.h>
#include<signal.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/shm.h>
#include<unistd.h>
#include<sys/un.h>
#include<netinet/ip.h>
#include<arpa/inet.h>
#include<errno.h>
#include<netinet/if_ether.h>
#include<net/ethernet.h>
#include<netinet/ether.h>
#include<netinet/udp.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include <sys/sem.h>
#include<poll.h>
#include<pcap.h>

#include<bits/stdc++.h>
using namespace std;
struct Adata
{
  int fd;
  int hallTicketno;
};
map<int,vector<string>>Answers; 
set<int>Cheater;
int noOfQues=3;
void *fun(void *fd)
{
    struct Adata *temp=(struct Adata*)(fd);
    for(int i=0;i<noOfQues;i++)
    {
        char buff[100];
        recv(temp->fd,buff,sizeof(buff),0);
        string str(buff);
        Answers[temp->hallTicketno].push_back(str);
    }
    if(Cheater.count(temp->hallTicketno)==0)
    {
        char *buff="Succesful completion of exam";
        send(temp->fd,buff,strlen(buff),0);
    }
    close(temp->fd);
    free(temp);
}
int main()
{

    int rsfd = socket(AF_INET,SOCK_RAW,IPPROTO_RAW);
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
    // addr.sin_port=htons(8888);
	if(bind(rsfd,(struct sockaddr*)&addr,sizeof(addr))<0)
	{
		perror("Could not bind");
        close(rsfd);
        exit(0);
	}
   
    int sfd[5];
    for (int i = 0; i < 5; i++)
    {
        sfd[i] = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
        if (sfd[i] < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in serveaddr;
        serveaddr.sin_family = AF_INET;
        serveaddr.sin_port = htons(8080+i);
        serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (bind(sfd[i], (struct sockaddr *)&serveaddr, sizeof(serveaddr)) < 0) {
            perror("Bind failed");
            close(sfd[i]);
            exit(EXIT_FAILURE);
        }

        if (listen(sfd[i], 5) < 0) { // 5 is the backlog
            perror("Listen failed");
            close(sfd[i]);
            exit(EXIT_FAILURE);
        }
    }
    pthread_t tid[5];
    int n=0;
    struct pollfd PFD[6];
    for(int i=0;i<5;i++) PFD[i].fd=sfd[i];
    PFD[5].fd=rsfd;
    for(int i=0;i<6;i++) PFD[i].events=POLLIN;
    while(1)
    {
        int a=poll(PFD,6,-1);
        if(a>0)
        {
            for(int i=0;i<6;i++)
            {
                if(PFD[i].revents&POLLIN)
                {
                    if(i==5)
                    {
                        char buff[100];
                           struct sockaddr_in client_addr;
                           socklen_t addr_len = sizeof(client_addr);
                        if(recvfrom(PFD[i].fd,buff,sizeof(buff),0,(struct sockaddr*)&client_addr, &addr_len)<0){
                            perror("error in receiving");
                            exit(0);
                        }
                        Cheater.insert(atoi(buff));
                        cout<<"Cheater "<<buff<<endl;
                        fflush(stdout);
                    }
                    else{
                        struct sockaddr_in clientaddr;
                        socklen_t clientsize = sizeof(clientaddr);
                        int nsfd = accept(PFD[i].fd, (struct sockaddr *)&clientaddr, &clientsize);
                        if (nsfd < 0) {
                            perror("Accept failed");
                            continue;
                        }
                        char *Ques="Ques1:......,Ques2:......,Ques3.......\n";
                        send(nsfd,Ques,strlen(Ques),0);
                        char buff[100];
                        recv(nsfd,buff,sizeof(buff),0);
                        printf("%s\n",buff);
                        fflush(stdout);
                        struct Adata *Temp = (struct Adata *)malloc(sizeof(struct Adata));
                        Temp->fd = nsfd;
                        Temp->hallTicketno = atoi(buff);
                        n++;
                        pthread_create(&tid[i],NULL,fun,(void *)Temp);


                    }
                }
            }
        }
    }





}