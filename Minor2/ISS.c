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

#define ADDRESS  "mysocket"

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
 

 int recv_fd(int socket)
 {
  int sent_fd, available_ancillary_element_buffer_space;
  struct msghdr socket_message;
  struct iovec io_vector[1];
  struct cmsghdr *control_message = NULL;
  char message_buffer[1];
  char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];

  /* start clean */
  memset(&socket_message, 0, sizeof(struct msghdr));
  memset(ancillary_element_buffer, 0, CMSG_SPACE(sizeof(int)));

  /* setup a place to fill in message contents */
  io_vector[0].iov_base = message_buffer;
  io_vector[0].iov_len = 1;
  socket_message.msg_iov = io_vector;
  socket_message.msg_iovlen = 1;

  /* provide space for the ancillary data */
  socket_message.msg_control = ancillary_element_buffer;
  socket_message.msg_controllen = CMSG_SPACE(sizeof(int));

  if(recvmsg(socket, &socket_message, MSG_CMSG_CLOEXEC) < 0)
   return -1;

  if(message_buffer[0] != 'F')
  {
   /* this did not originate from the above function */
   return -1;
  }

  if((socket_message.msg_flags & MSG_CTRUNC) == MSG_CTRUNC)
  {
   /* we did not provide enough space for the ancillary element array */
   return -1;
  }

  /* iterate ancillary elements */
   for(control_message = CMSG_FIRSTHDR(&socket_message);
       control_message != NULL;
       control_message = CMSG_NXTHDR(&socket_message, control_message))
  {
   if( (control_message->cmsg_level == SOL_SOCKET) &&
       (control_message->cmsg_type == SCM_RIGHTS) )
   {
    sent_fd = *((int *) CMSG_DATA(control_message));
    return sent_fd;
   }
  }

  return -1;
 }

int main()
{
     int sfd = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
    if (sfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in serveaddr;
    serveaddr.sin_family = AF_INET;
    serveaddr.sin_port = htons(9090);
    serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sfd, (struct sockaddr *)&serveaddr, sizeof(serveaddr)) < 0) {
        perror("Bind failed");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sfd, 5) < 0) { // 5 is the backlog
        perror("Listen failed");
        close(sfd);
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in clientaddr;
    socklen_t clientsize = sizeof(clientaddr);
    int nsfd = accept(sfd, (struct sockaddr *)&clientaddr, &clientsize);
    if (nsfd < 0) {
        perror("Accept failed");
        exit(0);
    }
   
    struct pollfd PFD[100];
    int size=1;
    PFD[0].fd=nsfd;
    for(int i=0;i<100;i++){
        PFD[i].events=POLLIN;
    }

    int port[99];
    int usfds[99];
    while(1)
    {
        int a=poll(PFD,size,100);
        if(a>0)
        {
            for(int i=0;i<size;i++)
            {
                if(PFD[i].revents&POLLIN)
                {
                    if(i==0)
                    {
                        char buff[256];
                        if(recv(nsfd,buff,sizeof(buff),0)<0) perror("recv error");
                        // printf("1%s\n",buff);
                        // sleep(10);
                        // fflush(stdout);
                        port[size-1]=atoi(buff);

                         int nfd = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
                        if (nfd < 0) {
                            perror("Socket creation failed");
                            exit(EXIT_FAILURE);
                        }
                        int opt = 1;
                         setsockopt(nfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
                        struct sockaddr_in serveaddr;
                        serveaddr.sin_family = AF_INET;
                        serveaddr.sin_port = htons(port[size-1]);
                        serveaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

                        if (bind(nfd, (struct sockaddr *)&serveaddr, sizeof(serveaddr)) < 0) {
                            perror("Bind failed");
                            close(nfd);
                            exit(EXIT_FAILURE);
                        }

                        if (listen(nfd, 5) < 0) { // 5 is the backlog
                            perror("Listen failed");
                            close(nfd);
                            exit(EXIT_FAILURE);
                        }
                        PFD[size].fd=nfd;

                          int usfd;
                        struct sockaddr_un userv_addr;
                        int userv_len;
                        // Create Unix domain socket
                        usfd = socket(AF_UNIX, SOCK_STREAM, 0); // Change to SOCK_STREAM
                        if (usfd == -1)
                        {
                            perror("socket");
                            exit(1);
                        }
                        bzero(&userv_addr, sizeof(userv_addr));
                        userv_addr.sun_family = AF_UNIX;
                        strcpy(userv_addr.sun_path, buff);
                        userv_len = sizeof(userv_addr);
                        // Connect to the server
                        if (connect(usfd, (struct sockaddr *)&userv_addr, userv_len) < 0)
                        {
                            perror("connect: usfd");
                            exit(1);
                        }
                        usfds[size-1]=usfd;

                        size++;
                    }
                    else{
                         struct sockaddr_in clientaddr;
                        socklen_t clientsize = sizeof(clientaddr);
                        int tempfd = accept(PFD[i].fd, (struct sockaddr *)&clientaddr, &clientsize);
                        if (tempfd < 0) {
                            perror("Accept failed");
                            exit(0);
                        }
                        send_fd(usfds[i-1],tempfd);
                    }
                }
            }
        }
    }


  
    printf("connected usfd\n");


}