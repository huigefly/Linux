#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <list>
#include <string>
using namespace std;

#define MAX_BUF_SIZE 100
list<string> g_list;
pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

void *proc_producer(void *lparam)
{
    int i = 0;
    unsigned int tid =pthread_self();
    while(1){
        char szBuf[64] = {0};
        sprintf(szBuf, "producer_[%u]_[%d]", tid, i++);
        pthread_mutex_lock(&g_lock);
        if (g_list.size() <= MAX_BUF_SIZE) {
            g_list.push_back(szBuf); 
        }
        pthread_mutex_unlock(&g_lock);
        printf("proc_producer tid:%u, buf:%s, list size:%lu\n", tid, szBuf, g_list.size());
        sleep(1);
    }
}

void *proc_consumer(void *lparam)
{
    while(1){
        if (g_list.size() > 0) {
            printf(" proc_consumer get:%s\n", g_list.front().c_str());
            g_list.pop_front();
        }
        printf(" proc_consumer list size:%lu\n", g_list.size());
        sleep(2);
    }
}

int main()
{

    pthread_t tid_consumer, tid_producer;
    int i = 0;
    for (i=0; i<10; i++){
        if (-1 == pthread_create(&tid_producer, NULL, proc_producer, NULL)){
            return -1;
        }
    }

    if (-1 == pthread_create(&tid_consumer, NULL, proc_consumer, NULL)){
        return -1;
    }

    sleep (520 * 1314);

    return 0;
}