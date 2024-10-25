#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <sys/select.h>
#include <signal.h>
#include <netinet/ip.h>
#include <sys/stat.h>
#include <errno.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/udp.h>
#include <ifaddrs.h>
#include <netdb.h>
#define TCP 1
#define UDP 2
// BSD Sockets
void getmyip(char *ip)
{
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    // Get a linked list of network interfaces
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return;
    }

    // Loop through each network interface
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue; // Skip if there's no address

        // Only consider IPv4 addresses
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            // Convert the network address to a readable format
            if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0)
            {
                // Skip the loopback interface (127.0.0.1)
                if (strcmp(host, "127.0.0.1") != 0)
                {
                    strcpy(ip, host);
                    return;
                }
            }
        }
    }

    // Free the linked list
    freeifaddrs(ifaddr);

    return;
}
struct sockaddr_in baddr_maker(char *ip, int port)
{
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    return addr;
}

int bserver_sock(int port, int protocol)
{
    int sfd;
    if (protocol == TCP)
        sfd = socket(AF_INET, SOCK_STREAM, 0);
    else
        sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0)
    {
        perror("Socket making failed");
    }
    char ip[100];
    getmyip(ip);
    struct sockaddr_in addr = baddr_maker(ip, port);
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        perror("BSD bind failed");
    if (protocol == TCP)
    {
        if (listen(sfd, 5) < 0)
            perror("BSD listen failed");
    }
    return sfd;
}

int bclient_sock(int port, int protocol)
{
    int sfd;
    if (protocol == TCP)
        sfd = socket(AF_INET, SOCK_STREAM, 0);
    else
        sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd < 0)
    {
        perror("Socket making failed");
    }
    char ip[100];
    getmyip(ip);
    struct sockaddr_in addr = baddr_maker(ip, port);
    socklen_t s = sizeof(addr);
    if (protocol == TCP)
    {
        if (connect(sfd, (struct sockaddr *)&addr, s) < 0)
            perror("Client Connect failed");
    }
    else
    {
        if (bind(sfd, (struct sockaddr *)&addr, s) < 0)
            perror("Bind socket UDP failed");
    }
    return sfd;
}

int bclient_addr_sock(struct sockaddr_in addr)
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        perror("Socket making failed");
    }
    socklen_t s = sizeof(addr);
    if (connect(sfd, (struct sockaddr *)&addr, s) < 0)
        perror("Client Connect failed");
    return sfd;
}

void baddr_to_ip_port(struct sockaddr_in addr, int *arr)
{ // arr is a 5 size arr to get n1.n2.n3.n4 n5(port)
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    int t = 0, j = 0;
    strcat(ip, ".");
    for (int i = 0; i < strlen(ip); i++)
    {
        if (ip[i] == '.')
        {
            arr[j] = t;
            t = 0;
            j++;
        }
        else
        {
            t *= 10;
            t += ip[i] - '0';
        }
    }
    arr[j] = ntohs(addr.sin_port);
}
int main(){
	int sfd=bclient_sock(9000,TCP);
	char buf[100];
	int n=5;
	sprintf(buf,"%d",n);
	if(send(sfd,buf,sizeof(buf),0)<0){
	 perror("send: ");
	}
  
	char **coupon=(char**)malloc(n*sizeof(char*));
	for(int i=0;i<n;i++){
		coupon[i]=(char*)malloc(100*sizeof(char));
	}
	for(int i=0;i<n;i++)
	{
	 if(recv(sfd,coupon[i],100,0)<0){
	 
	  perror("receive: ");
	 }
     else{
        printf("%s\n",coupon[i]);
     }
	}
	int rsfd=socket(AF_INET,SOCK_RAW,147);
    if(rsfd<0)
        perror("raw socket fail: ");
	struct sockaddr_in addr;
	socklen_t s=sizeof(addr);
    memset(buf,0,sizeof(buf));
	if(recvfrom(rsfd,buf,100,0,(struct sockaddr*)&addr,&s)<0)
        perror("recvfrom winner coupon: ");
	int b=0;
    struct iphdr* iph=(struct iphdr*)buf;
    printf("Winner coupon is: %s\n",buf+(iph->ihl*4));
	for(int i=0;i<n;i++){
		if(strcmp(coupon[i],buf+(iph->ihl*4))==0){
			b=1;
			break;
		}
	}
	if(b)
	sprintf(buf,"Winner!");
	else
	sprintf(buf,"Loser!");
	if(send(sfd,buf,sizeof(buf),0)<0){
	 perror("send: ");
	}
    else{
        printf("%s\n",buf);
    }
    memset(buf,0,sizeof(buf));
    iph=(struct iphdr*)buf;
	if(recvfrom(rsfd,buf,100,0,(struct sockaddr*)&addr,&s)<0)
        perror("recvfrom winner result: ");
	printf("%s\n",buf+iph->ihl*4);
}