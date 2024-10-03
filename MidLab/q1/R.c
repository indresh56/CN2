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
#define ADDRESS  "mysocket"
#define ADDRESS2 "mysockett"
int main()
{
    int sfds = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfds == -1) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in serveraddress;
    socklen_t serverlen = sizeof(serveraddress);
    memset(&serveraddress, 0, sizeof(serveraddress)); // Zero out the structure
    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(7075);
    serveraddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    // if (bind(sfds, (struct sockaddr *)&serveraddress, sizeof(serveraddress)) == -1) {
    //     perror("Bind failed");
    //     close(sfds);
    //     return 1;
    // }
    //  int sfds = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
    //     if (sfds < 0) {
    //         perror("Socket creation failed");
    //         exit(EXIT_FAILURE);
    //     }

    //     // Define server address
    //     struct sockaddr_in serveaddrs;
    //     serveaddrs.sin_family = AF_INET;
    //     serveaddrs.sin_port = htons(7075); // change port no for different processes
    //     serveaddrs.sin_addr.s_addr = inet_addr("127.0.0.1");

    //     // Connect to the server
    //     if (connect(sfds, (struct sockaddr *)&serveaddrs, sizeof(serveaddrs)) < 0) {
    //         perror("Connection to server failed");
    //         close(sfds);
    //         exit(EXIT_FAILURE);
    //     }

    
    int sfd1,sfd2;
    sfd1 = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
        if (sfd1 < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }
    sfd2 = socket(AF_INET, SOCK_STREAM, 0); // 0 given for default
    if (sfd2 < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
   
         struct sockaddr_in serveaddr1;
        serveaddr1.sin_family = AF_INET;
        serveaddr1.sin_port = htons(7070);
        serveaddr1.sin_addr.s_addr = inet_addr("127.0.0.1");
        if (bind(sfd1, (struct sockaddr *)&serveaddr1, sizeof(serveaddr1)) < 0) {
            perror("Bind failed");
            close(sfd1);
            exit(EXIT_FAILURE);
        }
         if (listen(sfd1, 5) < 0) { // 5 is the backlog
            perror("Listen failed");
            close(sfd1);
            exit(EXIT_FAILURE);
        }
        struct sockaddr_in serveaddr2;
        serveaddr2.sin_family = AF_INET;
        serveaddr2.sin_port = htons(7071);
        serveaddr2.sin_addr.s_addr = inet_addr("127.0.0.1");
        if (bind(sfd2, (struct sockaddr *)&serveaddr2, sizeof(serveaddr2)) < 0) {
            perror("Bind failed");
            close(sfd2);
            exit(EXIT_FAILURE);
        }
         if (listen(sfd2, 5) < 0) { // 5 is the backlog
            perror("Listen failed");
            close(sfd2);
            exit(EXIT_FAILURE);
        }
    
    

        struct pollfd P[2];
        P[0].fd=sfd1;
        P[1].fd=sfd2;
        // P[2].fd=usfd;
        P[0].events=POLLIN;
        P[1].events=POLLIN;
        // P[2].events=POLLIN;
        while(1)
        {
            int a=poll(P,2,-1);
            if(a>0)
            {
                for(int i=0;i<2;i++)
                {

                    if(P[i].revents&POLLIN)
                    {
                        struct sockaddr_in clientaddr;
                        socklen_t clientsize = sizeof(clientaddr);
                        int nsfd = accept(P[i].fd, (struct sockaddr *)&clientaddr, &clientsize);
                        if (nsfd < 0) {
                            perror("Accept failed");
                            continue;
                        }
                            struct sockaddr_in addrip;
                            socklen_t addr_lenip = sizeof(addrip);

                            if (getpeername(nsfd, (struct sockaddr *)&addrip, &addr_lenip) == -1) {
                                perror("getpeername failed");
                                return -1;
                            }

                            char ip[INET_ADDRSTRLEN];
                            inet_ntop(AF_INET, &(addrip.sin_addr), ip, sizeof(ip));
                            char buff[100];
                            sprintf(buff,"Connected to %s:%d\n", ip, ntohs(addrip.sin_port));
                            printf("%s",buff);
                            fflush(stdout);
                                if (sendto(sfds, buff, strlen(buff), 0, (struct sockaddr *)&serveraddress, serverlen) == -1) {
                                    perror("Sendto failed");
                                    close(sfds);
                                    return 1;
                                }

                            // send(sfds,buff,strlen(buff),0); 
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

                            int  usfd2;
                            struct sockaddr_un userv_addr2,ucli_addr2;
                            int userv_len2,ucli_len2;
                            usfd2 = socket(AF_UNIX , SOCK_STREAM , 0);
                            perror("socket");
                            bzero(&userv_addr2,sizeof(userv_addr2));
                            userv_addr2.sun_family = AF_UNIX;
                            strcpy(userv_addr2.sun_path, ADDRESS2);
                            unlink(ADDRESS2);
                            userv_len2 = sizeof(userv_addr2);

                            if(bind(usfd2, (struct sockaddr *)&userv_addr2, userv_len2)==-1)
                            perror("server: bind");
                            if(listen(usfd2, 10) < 0) {
                                perror("listen");
                                exit(1);
                            }


                        if(i==0)
                        {
                            int nufd;
                            if((nufd = accept(usfd, (struct sockaddr*)&ucli_addr, &ucli_len)) < 0) {
                                perror("accept");
                                exit(1);
                            }
                            send_fd(nufd,nsfd);
                            close(nufd);
                        }
                        else{
                            int nufd;
                            if((nufd = accept(usfd2, (struct sockaddr*)&ucli_addr2, &ucli_len2)) < 0) {
                                perror("accept");
                                exit(1);
                            }
                            send_fd(nufd,nsfd);
                            close(nufd);
                        }
                        close(nsfd);
                        close(usfd);
                        close(usfd2);
                        
                    }
                }
            }
        }

}
    
