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

void sem_wait(int semid, int smno) {
    struct sembuf sb;
    sb.sem_num = smno;
    sb.sem_op = -1;
    sb.sem_flg = 0;
    if (semop(semid, &sb, 1) == -1) {
        perror("Failed to acquire semaphore");
        exit(EXIT_FAILURE);
    }
}

int sem_try_wait(int semid, int smno) {
    struct sembuf sb;
    sb.sem_num = smno;
    sb.sem_op = -1;
    sb.sem_flg = IPC_NOWAIT;
    return semop(semid, &sb, 1);
}

void sem_signal(int semid, int smno) {
    struct sembuf sb;
    sb.sem_num = smno;
    sb.sem_op = 1;
    sb.sem_flg = 0;
    if (semop(semid, &sb, 1) == -1) {
        perror("Failed to release semaphore");
        exit(EXIT_FAILURE);
    }
}

int main() {
    key_t key;

    // Get key for shared memory 1
    key = ftok("x", 65);
    if (key == -1) {
        perror("ftok failed for key 'x'");
        exit(EXIT_FAILURE);
    }

    // Get shared memory 1
    int shmid1 = shmget(key, sizeof(int), IPC_CREAT | 0666);
    if (shmid1 == -1) {
        perror("shmget failed for shared memory 1");
        exit(EXIT_FAILURE);
    }

    // Get key for shared memory 2
    key = ftok("y", 65);
    if (key == -1) {
        perror("ftok failed for key 'y'");
        exit(EXIT_FAILURE);
    }

    // Get shared memory 2
    int shmid2 = shmget(key, sizeof(int), IPC_CREAT | 0666);
    if (shmid2 == -1) {
        perror("shmget failed for shared memory 2");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory 1
    int *x = shmat(shmid1, (const void*)0, 0);
    if (x == (void *) -1) {
        perror("shmat failed for shared memory 1");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory 2
    int *y = shmat(shmid2, (const void*)0, 0);
    if (y == (void *) -1) {
        perror("shmat failed for shared memory 2");
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory values
    *x = 1;
    *y = 1;

    // Get key for semaphore
    int state = 1;
    key = ftok(".", state++);
    if (key == -1) {
        perror("ftok failed for semaphore");
        exit(EXIT_FAILURE);
    }

    // Get semaphore set
    int sem_id = semget(key, 2, 0666 | IPC_CREAT);
    if (sem_id == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }

    // Set initial semaphore values
    int semvalue = 0;
    if (semctl(sem_id, 0, SETVAL, semvalue) == -1) {
        perror("semctl failed for semaphore 0");
        exit(EXIT_FAILURE);
    }

    if (semctl(sem_id, 1, SETVAL, semvalue) == -1) {
        perror("semctl failed for semaphore 1");
        exit(EXIT_FAILURE);
    }

    int temp = 1;

    // Example loop
    for (int i = 0; i < 5; i++) {
        *x = temp;

        // Signal semaphore 0
        sem_signal(sem_id, 0);

        // Wait on semaphore 1
        sem_wait(sem_id, 1);

        // Output the value of y
        printf("y: %d\n", *y);

        // Increment temp by the value of y
        temp = *y + 1;
    }

    return 0;
}

// #include<time.h>
// #include<stdio.h>
// #include<sys/socket.h>
// #include<netinet/in.h>
// #include<string.h>
// #include<sys/select.h>
// #include<pthread.h>
// #include<signal.h>
// #include<stdlib.h>
// #include<fcntl.h>
// #include<sys/shm.h>
// #include<unistd.h>
// #include<sys/un.h>
// #include<netinet/ip.h>
// #include<arpa/inet.h>
// #include<errno.h>
// #include<netinet/if_ether.h>
// #include<net/ethernet.h>
// #include<netinet/ether.h>
// #include<netinet/udp.h>
// #include<sys/ipc.h>
// #include<sys/msg.h>
// #include <sys/sem.h>
// void sem_wait(int semid,int smno)
// 	{
// 		struct sembuf sb;
// 		sb.sem_num=smno;
// 		sb.sem_op=-1;
// 		sb.sem_flg=0;
// 		if((semop(semid,&sb,1))==-1)
// 		{
// 			perror("\nFailed to acquire semaphore.");
// 			exit(0);
// 		}
// 	}

// 	void sem_try_wait(int semid,int smno)
// 	{
// 		struct sembuf sb;
// 		sb.sem_num=smno;
// 		sb.sem_op=-1;
// 		sb.sem_flg=IPC_NOWAIT;;
// 		return semop(semid,&sb,1);
// 	}

// 	void sem_signal(int semid,int smno)
// 	{
// 		struct sembuf sb;
// 		sb.sem_num=smno;
// 		sb.sem_op=1;
// 		sb.sem_flg=0;
// 		if((semop(semid,&sb,1))==-1)
// 		{
// 			perror("\nFailed to release semaphore.");
// 			exit(0);
// 		}
// 	}

// int main()
// {
//   key_t key=ftok("x",65);
//   int shmid1=shmget(key,sizeof(int),IPC_CREAT|0666);
//   key=ftok("y",65);
//   int shmid2=shmget(key,sizeof(int),IPC_CREAT|0666);
//   int *x=shmat(shmid1,(const void*)0,0);
//   int *y=shmat(shmid2,(const void*)0,0);
//   *x=1;*y=1;
//   int state=1;
//   key=ftok(".",state++);
//   int sem_id=semget(key,2,0666|IPC_CREAT);
//   int semvalue=0;
//   semctl(sem_id, 0, SETVAL, semvalue);
//   semctl(sem_id, 1, SETVAL, semvalue);
//   int temp=1;
// //   while(1)
//  for(int i=0;i<5;i++)
//   {
   
//     *x=temp;
//     sem_signal(sem_id,0);
//     sem_wait(sem_id,1);
//      printf("y: %d\n",*y);
//     temp=*y+1;

//   }

// }
