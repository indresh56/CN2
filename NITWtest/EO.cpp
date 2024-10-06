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
#include <sys/types.h> 
#include <sys/stat.h> 
#include<bits/stdc++.h>
struct sockaddr_in addr;
int rsfd;
using namespace std;

int main()
{
    rsfd = socket(AF_INET,SOCK_RAW,75);
	if(rsfd<0)
	{
		perror("Could not create socket");exit(0);
	}

	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port=htons(8888);
	
    if(mkfifo("fifo",0666)==-1)//fifo for sending warning
    {
        perror("fifo creation");
    }
    int ffd=open("fifo",O_WRONLY);
    
    int rsfd2 = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
	if(rsfd2<0)
	{
		perror("Could not create socket");exit(0);
	}
	char buffer[65536];
	if(setsockopt(rsfd,SOL_SOCKET,SO_BINDTODEVICE,"eth0",strlen("eth0"))<0)
	{
		perror("Could not set option");
	}
	int sz;
    int count=0;
	while(1)
	{
		sz = recvfrom(rsfd2,buffer,65536,0,NULL,NULL);
		if(sz<0)
		{
			perror("Reading error");
		}
		else
		{
			
			struct iphdr* ip;
            struct ether_header* eth;
			eth = (struct ether_header*)buffer;

			if(ntohs(eth->ether_type)==ETHERTYPE_IP)
			{
				ip = (struct iphdr*)(buffer+14);
                cout<<"Source ip address "<<inet_ntoa(*(in_addr*)&ip->saddr)<<" Destination address "<<inet_ntoa(*(in_addr*)&ip->daddr)<<endl;
                fflush(stdout);
				if(strcmp(inet_ntoa(*(in_addr*)&ip->saddr),"255.255.255.255")==0&&strcmp(inet_ntoa(*(in_addr*)&ip->daddr),"172.21.96.1")==0) continue;
                else if(strcmp(inet_ntoa(*(in_addr*)&ip->saddr),"172.21.96.1")==0&&strcmp(inet_ntoa(*(in_addr*)&ip->daddr),"255.255.255.255")==0) continue;
                count++;
                if(count==1) {
                    char *str="Don't switch tabs or open any program: First time warning\n";
                    write(ffd,str,strlen(str));
                    }
                    else if(count==2)
                    {
                        char *str="Don't switch tabs or open any program: Last time warning\n";
                    write(ffd,str,strlen(str));
                    }
                    else{
                        char *str="8080";
                        if(sendto(rsfd,str,strlen(str),0,(struct sockaddr*)&addr, sizeof(addr))<0){
                            perror("send failed");
                        };
                        break;

                    }
                
			}

			
		}
	}
    close(rsfd);
    close(ffd);
    


    

}