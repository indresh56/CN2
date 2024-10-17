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
    // rsfd = socket(AF_INET,SOCK_RAW,IPPROTO_UDP);
	// if(rsfd<0)
	// {
	// 	perror("Could not create socket");exit(0);
	// }
    int sfd=socket(AF_INET,SOCK_DGRAM,0);
    if(sfd<0)
	{
		perror("Could not create socket");exit(0);
	}
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
    // addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port=htons(8888);
	// 	int optval=1;
	// setsockopt(rsfd, IPPROTO_IP, SO_BROADCAST, &optval, sizeof(int));
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
	if(setsockopt(rsfd2,SOL_SOCKET,SO_BINDTODEVICE,"eth0",strlen("eth0"))<0)
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
                    else
                    {
                        // const char *message = "8080";
                        // int message_length = strlen(message);

                        // // Prepare the IP header
                        // struct iphdr *ip_header = (struct iphdr *)malloc(sizeof(struct iphdr) + message_length);
                        // ip_header->version = 4; // IPv4
                        // ip_header->ihl = 5; // Header length
                        // ip_header->tos = 0; // Type of service
                        // ip_header->tot_len = htons(sizeof(struct iphdr) + message_length); // Total length
                        // ip_header->id = htons(54321); // Identification
                        // ip_header->frag_off = 0; // Fragment offset
                        // ip_header->ttl = 64; // Time to live
                        // ip_header->protocol = IPPROTO_RAW; // Protocol
                        // ip_header->check = 0; // Checksum (0 for now)
                        // ip_header->saddr = inet_addr("127.0.0.1"); // Source IP address
                        // ip_header->daddr = inet_addr("127.0.0.1"); // Destination IP address

                        // // Copy the message to the buffer after the IP header
                        // memcpy((char *)ip_header + sizeof(struct iphdr), message, message_length);
                        // if (sendto(rsfd, ip_header, sizeof(struct iphdr) + message_length, 0,(struct sockaddr*)&addr, sizeof(addr)) < 0) {
                        //             perror("Send failed");
                        //  }
                         


                        char *str="8080";
                        if(sendto(sfd, str, strlen(str), 0, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                            perror("Send failed");
                            close(sfd);
                            exit(1);
                        }
                        
                        //system("ip link set eth0 down");                SHUTTING DOWN INTERNET






                        // if(sendto(rsfd,str,strlen(str),0,(struct sockaddr*)&addr, sizeof(addr))<0){
                        //     close(rsfd);
                        //     perror("send failed");
                        //      cout << "Error code: " << errno << endl;
                        // };
                        cout<<"Data sent"<<endl;
                        break;

                    }
                
			}

			
		}
	}
    close(rsfd);
    close(ffd);
    


    

}