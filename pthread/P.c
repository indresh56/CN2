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

//TWO CODES BELOW


/*#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>  // For errno
#include <string.h> // For strerror

void* fun(void *arg) {
    printf("Thread no %d\n", *((int*)arg));
    pthread_exit(NULL); // It's good practice to exit the thread properly
}

int main() {
    pthread_t tid1, tid2;
    int a = 1, b = 2;

    // Create the first thread
    int result1 = pthread_create(&tid1, NULL, fun, (void*)&a);
    if (result1 != 0) {
        fprintf(stderr, "Error creating thread 1: %s\n", strerror(result1));
        exit(EXIT_FAILURE);
    }

    // Create the second thread
    int result2 = pthread_create(&tid2, NULL, fun, (void*)&b);
    if (result2 != 0) {
        fprintf(stderr, "Error creating thread 2: %s\n", strerror(result2));
        exit(EXIT_FAILURE);
    }

    // Wait for the first thread to finish
    int join_result1 = pthread_join(tid1, NULL);
    if (join_result1 != 0) {
        fprintf(stderr, "Error joining thread 1: %s\n", strerror(join_result1));
        exit(EXIT_FAILURE);
    }

    // Wait for the second thread to finish
    int join_result2 = pthread_join(tid2, NULL);
    if (join_result2 != 0) {
        fprintf(stderr, "Error joining thread 2: %s\n", strerror(join_result2));
        exit(EXIT_FAILURE);
    }

    return 0;
}
 */




/*#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int thread_id;
    char message[20];
} thread_data_t;

void* thread_function(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;  // Cast the argument back to the original type
    printf("Thread ID: %d, Message: %s\n", data->thread_id, data->message);
    pthread_exit(NULL);  // Exit the thread
}

int main() {
    pthread_t threads[5];
    thread_data_t thread_data[5];  // Array to hold thread data

    for (int i = 0; i < 5; i++) {
        thread_data[i].thread_id = i;
        snprintf(thread_data[i].message, sizeof(thread_data[i].message), "Hello from %d", i);

        // Create a thread
        int rc = pthread_create(&threads[i], NULL, thread_function, (void*)&thread_data[i]);
        if (rc) {
            fprintf(stderr, "Error creating thread %d: %d\n", i, rc);
            exit(-1);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
 */