#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <list>
#include <string>
#include <semaphore.h>
using namespace std;

#define MAX_BUF_SIZE 100
string g_buf;

list<string> g_list;
pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

void *proc_reader(void *lparam)
{
    int i = 0;
    unsigned int tid =pthread_self();
    while (1) {
        pthread_mutex_lock(&g_lock);
        printf ("[%u] reader:%s\n", tid, g_buf.c_str());
        pthread_mutex_unlock(&g_lock);
        sleep(5);
    }
}

void *proc_writer(void *lparam)
{
    unsigned int tid =pthread_self();
    while (1) {
        pthread_mutex_lock(&g_lock);
        g_buf = to_string(tid) + "xxxxxxxxxxxxxxxxxxx";
        printf ("[%u] write:%s\n", tid, g_buf.c_str());
        pthread_mutex_unlock(&g_lock);
        sleep(5);
    }
}

int main()
{
    pthread_t tid_reader, tid_writer;
    int i = 0;
    for (i=0; i<10; i++){
        if (-1 == pthread_create(&tid_reader, NULL, proc_reader, NULL)){
            return -1;
        }
    }

    for (i=0; i<10; i++){
        if (-1 == pthread_create(&tid_writer, NULL, proc_writer, NULL)){
            return -1;
        }
    }

    sleep (520 * 1314);

    return 0;
}