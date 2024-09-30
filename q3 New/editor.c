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
struct msg
{
     long type;
    char message[200];
   
};
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
#define ADDRESS  "mysocket"
int main()
{
    mkfifo("Rep", 0666);
    
    mkfifo("DocWr",0666);
    // key_t key=ftok("news",65);
    key_t key = ftok("news", 65);
    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }
    int msqid=msgget(key,0666|IPC_CREAT);
    //printf("%d",msqid);
    int rfd[1];
    rfd[0] = open("Rep", O_RDWR);
    int dfd=open("DocWr",O_RDWR);
    struct pollfd prfd[3];
    for (int i = 0; i < 1; i++)
    {
        prfd[i].fd = rfd[i];
        prfd[i].events = POLLIN;
    }
    int whichnewsreader=1;
    while (1)
    {
        int a = poll(prfd, 1, 100);
        if (a > 0)
        {
            for (int i = 0; i < 1; i++)
            {
                if (prfd[i].revents & POLLIN)
                {
                    char s[2];
                    ssize_t bytesRead = read(prfd[i].fd, s, 2);
                   
                    if (bytesRead <= 0)
                    {
                        continue;
                    }
                    // read(prfd[i].fd, s, sizeof(s));
                    else
                    {
                        s[bytesRead]='\0';
                        if(s[0]=='/'&&s[1]=='d')
                        {
                             int  usfd;
                            struct sockaddr_un userv_addr,ucli_addr;
                            int userv_len,ucli_len;
                            usfd = socket(AF_UNIX , SOCK_STREAM , 0);
                            perror("socket");
                            bzero(&userv_addr,sizeof(userv_addr));
                            userv_addr.sun_family = AF_UNIX;
                            strcpy(userv_addr.sun_path, ADDRESS);
                            unlink(ADDRESS);
                            userv_len = sizeof(userv_addr);

                            if(bind(usfd, (struct sockaddr *)&userv_addr, userv_len)==-1)
                                perror("server: bind");
                            if(listen(usfd, 10) < 0) {
                                    perror("listen");
                                    exit(1);
                                }
                            int nufd;
                            if((nufd = accept(usfd, (struct sockaddr*)&ucli_addr, &ucli_len)) < 0) {
                                perror("accept");
                                exit(1);
                            }
                            send_fd(nufd,prfd[i].fd);
                            close(usfd);
                            // char str[198];
                            // for(int i=0;i<198;i++) str[i]=s[i+2];
                            // write(dfd,str,bytesRead);
                            //pass fd


                        }
                        else{
                             
                           
                               char ss[198];
                    ssize_t bytesRead = read(prfd[i].fd, ss, 198);
                               char FullS[200]; 
                               strcat(FullS,s);
                               strcat(FullS,ss);
                                struct msg M;
                                M.type=whichnewsreader;
                                 strncpy(M.message, FullS, sizeof(M.message) - 1);
                                M.message[sizeof(M.message) - 1] = '\0';
                                // strcpy(M.message,s);
                                // M.message[strlen(s)] = '\0'; 
                                printf("%s",M.message);
                                fflush(stdout);
                                if (msgsnd(msqid, &M, sizeof(M.message), 0) == -1) {
                                    perror("msgsnd failed");}
                                //msgsnd(msqid,&M,sizeof(M.message),0);
                                if(whichnewsreader==1)
                                whichnewsreader=2;
                            else whichnewsreader=1;

                           
                                fflush(stdout);

                            
                            
                            
                        }
                        //printf("%s\n", s);
                    }
                }
            }
        }
    }
}