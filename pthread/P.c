#include <time.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/udp.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
void* fun(void *arg)
{
   printf("Thread no %d\n",*((int*)arg));
}
int main()
{
 pthread_t tid1,tid2;
    int a=1,b=2;
    pthread_create(&tid1,NULL,fun,(void*)&a);
    pthread_create(&tid2,NULL,fun,(void*)&b);

    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);
}