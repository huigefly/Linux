#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

int g_value=0;

#define PTHREAD_SIZE 10
typedef struct {
    int m_nIndex;
    char m_buf[64];
}T_PARAM;


void *proc(void *lparam){
    int nIndex = *(int*)lparam;
    g_value++;
    while (1) {
        // printf("g_value:%d, thread[%d]: %ld, pid:%u, ppid:%u\n", g_value, nIndex, pthread_self(), getpid(), getppid());
        printf("i am value[%d], index[%d]: tid[%ld], pid:%u, ppid:%u\n", g_value, nIndex, pthread_self(), getpid(), getppid());
        
        #if 1
        // if (3 == nIndex){
            nIndex = 9999999;
            int pid = fork();
            if (pid == 0){
                
                printf("hello from the child, pid:%d, ppid:%u, value:[%d]\n", getpid(), getppid(), g_value);
                g_value=10256;
                sleep(3000000);
            }
            else{
                printf("hello from the parent pid:%d, ppid:%u\n", getpid(), getppid());
                // exit(0);
            }
        // }
        #endif //0
        sleep(10);
    }

}

int main()
{
    printf("helloworld\n");
    int nArr[PTHREAD_SIZE];
    pthread_t tid[PTHREAD_SIZE];
    for (int i=0; i<PTHREAD_SIZE; i++) {
        nArr[i] = i;
        if (0 != pthread_create(&tid[i], NULL, proc, &nArr[i])) {
            printf("pthread create error!\n");
        }
    }
    #if 0
    int pid = fork();
    if (pid == 0){
        printf("hello from the child, pid:%d\n", getpid());
        sleep(30);
    } 
    else{
        printf("hello from the parent pid:%d\n", getpid());
    }
    #endif //0

    for (int i=0; i<PTHREAD_SIZE; i++) {
        pthread_join(tid[i], NULL);
    }

    return 0;
}
