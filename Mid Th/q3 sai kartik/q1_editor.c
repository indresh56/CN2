#include<stdio.h>
#include<pthread.h>
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
//#include<pcap.h>
#include<errno.h>
#include<netinet/if_ether.h>
#include<net/ethernet.h>
#include<netinet/ether.h>
#include<netinet/udp.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#define SOCKET_PATH  "/tmp/mysocket"
int send_fd(int socket, int fd_to_send)
{
    struct msghdr socket_message;
    struct iovec io_vector[1];
    struct cmsghdr *control_message = NULL;
    char message_buffer[1];
    /* storage space needed for an ancillary element with a paylod of length is CMSG_SPACE(sizeof(length)) */
    char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];
    int available_ancillary_element_buffer_space;

    /* at least one vector of one byte must be sent */
    message_buffer[0] = 'F';
    io_vector[0].iov_base = message_buffer;
    io_vector[0].iov_len = 1;

    /* initialize socket message */
    memset(&socket_message, 0, sizeof(struct msghdr));
    socket_message.msg_iov = io_vector;
    socket_message.msg_iovlen = 1;

    /* provide space for the ancillary data */
    available_ancillary_element_buffer_space = CMSG_SPACE(sizeof(int));
    memset(ancillary_element_buffer, 0, available_ancillary_element_buffer_space);
    socket_message.msg_control = ancillary_element_buffer;
    socket_message.msg_controllen = available_ancillary_element_buffer_space;

    /* initialize a single ancillary data element for fd passing */
    control_message = CMSG_FIRSTHDR(&socket_message);
    control_message->cmsg_level = SOL_SOCKET;
    control_message->cmsg_type = SCM_RIGHTS;
    control_message->cmsg_len = CMSG_LEN(sizeof(int));
    *((int *) CMSG_DATA(control_message)) = fd_to_send;

    return sendmsg(socket, &socket_message, 0);
}
struct sockaddr_un userv_addr,ucli_addr;
int sfd,nsfd;
int sfd_news;
int  usfd;
int nusfd;
int userv_len,ucli_len;
struct sockaddr_in server_addr,client_addr;
socklen_t socklen_server;
void* send_to_reader(void* args)
{
    int nsfd1=nsfd;
    char buffer[1024];
    recv(nsfd1,buffer,2,0);
    if(buffer[0]=='/' && buffer[1]=='d')
    {
        nusfd=accept(usfd,NULL,NULL);
        if(nusfd<0)
        {
            perror("accept");
            close(usfd);
            exit(EXIT_FAILURE);
        }
        send_fd(nusfd,nsfd1);
    }
    else
    {
        recv(nsfd1,buffer,sizeof(buffer),0);
        send(sfd_news,buffer,sizeof(buffer),0);
    }
}
int main()
{
    sfd=socket(AF_INET,SOCK_STREAM,0);
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=INADDR_ANY;
	server_addr.sin_port=htons(9000);
	socklen_server=sizeof(server_addr);
	if(bind(sfd,(struct sockaddr*)&server_addr,socklen_server)<0) 
	{
		printf("Error in binding");
		return 0;
	}
    listen(sfd,5);

    
    sfd_news=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in server_addr1;
	server_addr1.sin_family=AF_INET;
	server_addr1.sin_addr.s_addr=inet_addr("127.0.0.1");
	server_addr1.sin_port=htons(10000);
	socklen_t socklen=sizeof(server_addr1);
	connect(sfd_news,(struct sockaddr*)&server_addr1,socklen);



	usfd = socket(AF_UNIX , SOCK_STREAM , 0);
    if(usfd<0)
	{
        perror("socket");
        exit(EXIT_FAILURE);
    }
    memset(&userv_addr, 0, sizeof(struct sockaddr_un));
  	userv_addr.sun_family = AF_UNIX;
	strncpy(userv_addr.sun_path, SOCKET_PATH,sizeof(userv_addr.sun_path)-1);
	unlink(SOCKET_PATH);
	userv_len = sizeof(userv_addr);
	if(bind(usfd, (struct sockaddr *)&userv_addr, userv_len)==-1)
	{
        perror("server: bind");
        close(usfd);
        exit(EXIT_FAILURE);
    }
	if(listen(usfd,5)<0)
    {
        perror("listen");
        close(usfd);
        exit(EXIT_FAILURE);
    }
	ucli_len=sizeof(ucli_addr);
	
	while(1)
	{
		socklen_t socklen_client=sizeof(client_addr);
        nsfd=accept(sfd,(struct sockaddr*)&client_addr,&socklen_client);
        if(nsfd<0)
        {
            printf("Error in accepting request");
            return 0;
        }
        else
        {
            pthread_t tid;
            pthread_create(&tid,NULL,&send_to_reader,NULL);
            pthread_join(tid,NULL);
        }
	}
}