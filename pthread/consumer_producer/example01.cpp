#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <list>
#include <string>
using namespace std;

list<string> g_list;

void *proc_producer(void *lparam)
{
    int i = 0;
    while(1){
        char szBuf[64] = {0};
        sprintf(szBuf, "producer[%d]", i++);
        g_list.push_back(szBuf);
        printf("proc_producer %s\n", szBuf);
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
    if (-1 == pthread_create(&tid_producer, NULL, proc_producer, NULL)){
        return -1;
    }

    if (-1 == pthread_create(&tid_consumer, NULL, proc_consumer, NULL)){
        return -1;
    }

    sleep (520 * 1314);

    return 0;
}