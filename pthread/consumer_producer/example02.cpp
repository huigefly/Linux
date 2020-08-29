#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <list>
#include <string>
#include <semaphore.h>
using namespace std;

#define MAX_BUF_SIZE 100

list<string> g_list;
pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
sem_t g_semEmpty;   // 有多少个空槽位可以存数据
sem_t g_semFull;    // 已经放了多少个数据


void *proc_producer(void *lparam)
{
    int i = 0;
    unsigned int tid =pthread_self();
    while (1) {
        char szBuf[64] = {0};
        sprintf(szBuf, "producer_[%u]_[%d]", tid, i++);
        sem_wait(&g_semEmpty);
        pthread_mutex_lock(&g_lock);
        g_list.push_back(szBuf);
        pthread_mutex_unlock(&g_lock);
        sem_post(&g_semFull); 
        printf("proc_producer tid:%u, buf:%s, list size:%lu\n", tid, szBuf, g_list.size());
        sleep(1);
    }
}

void *proc_consumer(void *lparam)
{
    while (1) {
        sem_wait(&g_semFull);
        printf(" proc_consumer get:%s\n", g_list.front().c_str());
        pthread_mutex_lock(&g_lock);
        g_list.pop_front();
        pthread_mutex_unlock(&g_lock);
        sem_post(&g_semEmpty);
        printf(" proc_consumer list size:%lu\n", g_list.size());
        usleep(1000 * 1);
    }
}

int main()
{
    sem_init(&g_semEmpty, 0, 100);
    sem_init(&g_semFull, 0, 0);

    pthread_t tid_consumer, tid_producer;
    int i = 0;
    for (i=0; i<10; i++){
        if (-1 == pthread_create(&tid_producer, NULL, proc_producer, NULL)){
            return -1;
        }
    }

    for (i=0; i<10; i++){
        if (-1 == pthread_create(&tid_consumer, NULL, proc_consumer, NULL)){
            return -1;
        }
    }

    sleep (520 * 1314);

    return 0;
}