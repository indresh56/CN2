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


#define ADDRESS  "mysocket"
#define ADDRESS2 "mysockett"
int num;
int sfds;
int first=-1,second=-1;
struct sockaddr_in serveraddress;
socklen_t serverlen;
void *fun(void *arg)
{
  int* fd=(int *)(arg); 
  char buff[20];
  recv(*fd,buff,sizeof(buff),0);
  int a=atoi(buff);
  if(a==num&&first==-1) {first=*fd;
   struct sockaddr_in addrip;
    socklen_t addr_lenip = sizeof(addrip);

    if (getpeername(first, (struct sockaddr *)&addrip, &addr_lenip) == -1) {
        perror("getpeername failed");
        exit(0);
    }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addrip.sin_addr), ip, sizeof(ip));
        char buff[100];
        sprintf(buff,"First %s:%d\n", ip, ntohs(addrip.sin_port));
        printf("%s",buff);
        fflush(stdout);
        if (sendto(sfds, buff, strlen(buff), 0, (struct sockaddr *)&serveraddress, serverlen) == -1) {
            perror("Sendto failed");
            close(sfds);
            exit(0);
        }
    }
  else if(a==num&&second==-1) {second=*fd;
  struct sockaddr_in addrip;
    socklen_t addr_lenip = sizeof(addrip);

    if (getpeername(second, (struct sockaddr *)&addrip, &addr_lenip) == -1) {
        perror("getpeername failed");
        exit(0);
    }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addrip.sin_addr), ip, sizeof(ip));
        char buff[100];
        sprintf(buff,"Second %s:%d\n", ip, ntohs(addrip.sin_port));
        printf("%s",buff);
        fflush(stdout);
        if (sendto(sfds, buff, strlen(buff), 0, (struct sockaddr *)&serveraddress, serverlen) == -1) {
            perror("Sendto failed");
            close(sfds);
            exit(0);
        }
    }

}

int main()
{

    num=rand()%26;
    printf("Random no %d\n",num);
    fflush(stdout);
    // int sfds = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
    // if (sfds < 0) {
    //     perror("Socket creation failed");
    //     exit(EXIT_FAILURE);
    // }

    // // Define server address
    // struct sockaddr_in serveaddrs;
    // serveaddrs.sin_family = AF_INET;
    // serveaddrs.sin_port = htons(7076); // change port no for different processes
    // serveaddrs.sin_addr.s_addr = inet_addr("127.0.0.1");

    // // Connect to the server
    // if (connect(sfds, (struct sockaddr *)&serveaddrs, sizeof(serveaddrs)) < 0) {
    //     perror("Connection to server failed");
    //     close(sfds);
    //     exit(EXIT_FAILURE);
    // }
     
    sfds = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfds == -1) {
        perror("Socket creation failed");
        return 1;
    }

    
     serverlen = sizeof(serveraddress);
    memset(&serveraddress, 0, sizeof(serveraddress)); 
    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(7075);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");
   
    
    int n=0,noOfplayer=2;

    //while(1)
    // struct pollfd P[1];
    // P[0].fd=usfd;
    // P[0].events=POLLIN;
    pthread_t threads[2];
    int pfds[2];
    while(1)
    {
          

         int usfd;
    struct sockaddr_un userv_addr;
    int userv_len;

    
    usfd = socket(AF_UNIX, SOCK_STREAM, 0); 
    if (usfd == -1) {
        perror("socket");
        close(usfd);
        exit(1);
    }

    bzero(&userv_addr, sizeof(userv_addr));
    userv_addr.sun_family = AF_UNIX;
    strcpy(userv_addr.sun_path, ADDRESS2);
    userv_len = sizeof(userv_addr);

    // Connect to the server
    if (connect(usfd, (struct sockaddr *)&userv_addr, userv_len) < 0) {
       close(usfd);
       continue;
    }
        int t=recv_fd(usfd);
        close(usfd);
       
        if(t==-1) continue;
        
        printf("%d\n",t);
        fflush(stdout);
        pfds[n++]=t;
        if(n==2) break;
        
        // int a=poll(P,1,100);
        // if(a>0&&P[0].revents&POLLIN)
        // {
        //     int t=recv_fd(usfd);
        //     printf("%d\n",t);
        // }
        //     int rc = pthread_create(&threads[i], NULL, fun, (void*)&t);
        // if (rc) {
        //     fprintf(stderr, "Error creating thread %d: %d\n", i, rc);
        //     exit(-1);
        // }

   
    }
  
    while(1)
    {
        for(int i=0;i<2;i++)
        {
          int rc = pthread_create(&threads[i], NULL, fun, (void*)&pfds[i]);
            if (rc) {
                fprintf(stderr, "Error creating thread %d: %d\n", i, rc);
                exit(-1);
            }
   
        }
        for(int i=0;i<2;i++)
        {
            pthread_join(threads[i], NULL);
        }
        if(first!=-1&&second!=-1) break;
    }
    printf("Hello");
    fflush(stdout);
   

        // if (getpeername(second, (struct sockaddr *)&addrip, &addr_lenip) == -1) {
        // perror("getpeername failed");
        // return -1;
        //  }

        
        // inet_ntop(AF_INET, &(addrip.sin_addr), ip, sizeof(ip));
       
        // sprintf(buff,"Second %s:%d\n", ip, ntohs(addrip.sin_port));
        // printf("%s",buff);
        // fflush(stdout);
        // if (sendto(sfds, buff, strlen(buff), 0, (struct sockaddr *)&serveraddress, serverlen) == -1) {
        //     perror("Sendto failed");
        //     close(sfds);
        //     return 1;
        // }
        // close(sfds);
        

}