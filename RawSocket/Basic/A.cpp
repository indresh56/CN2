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
#include<bits/stdc++.h>
using namespace std;
void print_ipheader(struct iphdr* ip)
{
	cout<<"------------------------\n";
	cout<<"Printing IP header....\n";
	cout<<"IP version:"<<(unsigned int)ip->version<<endl;
	cout<<"IP header length:"<<(unsigned int)ip->ihl<<endl;
	
	cout<<"Type of service:"<<(unsigned int)ip->tos<<endl;
	cout<<"Total ip packet length:"<<ntohs(ip->tot_len)<<endl;
	cout<<"Packet id:"<<ntohs(ip->id)<<endl;
	cout<<"Time to leave :"<<(unsigned int)ip->ttl<<endl;
	cout<<"Protocol:"<<(unsigned int)ip->protocol<<endl;
	cout<<"Check:"<<ip->check<<endl;
	cout<<"Source ip:"<<inet_ntoa(*(in_addr*)&ip->saddr)<<endl;
	//printf("%pI4\n",&ip->saddr );
	cout<<"Destination ip:"<<inet_ntoa(*(in_addr*)&ip->daddr)<<endl;
	cout<<"End of IP header\n";
	cout<<"------------------------\n";
}

int main(int argc, char const *argv[])
{
	int rsfd = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
	if(rsfd<0)
	{
		perror("Could not create socket");exit(0);
	}
	char buffer[65536];
	if(setsockopt(rsfd,SOL_SOCKET,SO_BINDTODEVICE,"eth0",strlen("eth0"))<0)
	{
		perror("Could not set option");
	}
	int sz;
	while(1)
	{
		sz = recvfrom(rsfd,buffer,65536,0,NULL,NULL);
		if(sz<0)
		{
			perror("Reading error");
		}
		else
		{
			printf("Reading %d bytes\n",sz);
			struct ether_header* eth;
			struct iphdr* ip;
			eth = (struct ether_header*)buffer;
			printf("******************************************\n\n");
			printf("Sender MAC Address: %s\n",ether_ntoa((const struct ether_addr*)eth->ether_shost));
			printf("Receiver MAC Address: %s\n",ether_ntoa((const struct ether_addr*)eth->ether_dhost));
			if(ntohs(eth->ether_type)==ETHERTYPE_IP)
			{
				ip = (struct iphdr*)(buffer+14);
                print_ipheader(ip);
				// printf("Version: %d\n",ip->version);
				// printf("Protocol: %d\n",ip->protocol);
				// printf("Type of Service: %d\n",ip->tos);
				// char ad[INET_ADDRSTRLEN],ad1[INET_ADDRSTRLEN];
				// int k = (ip->saddr);
				// printf("Sender IP: %s\n",inet_ntop(AF_INET,&k,ad,INET_ADDRSTRLEN));
				// k = (ip->daddr);
				// printf("Receiver IP: %s\n",inet_ntop(AF_INET,&k,ad1,INET_ADDRSTRLEN));
			}

			printf("******************************************\n\n");
		}
	}
	return 0;
}