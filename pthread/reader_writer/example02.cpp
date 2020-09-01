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
pthread_mutex_t g_lock_read = PTHREAD_MUTEX_INITIALIZER;
int g_read_count = 0;

void *proc_reader(void *lparam)
{
    int i = 0;
    unsigned int tid =pthread_self();
    while (1) {
        printf("reader g_lock_read lock\n");
        pthread_mutex_lock(&g_lock_read);
        g_read_count++;
        if (1 == g_read_count){
            printf("reader lock\n");
            pthread_mutex_lock(&g_lock);
        }
        printf("reader g_lock_read unlock\n");
        pthread_mutex_unlock(&g_lock_read);
        
        printf ("[%u] reader:%s\n", tid, g_buf.c_str());

        printf("reader g_lock_read lock 2\n");
        pthread_mutex_lock(&g_lock_read);
        g_read_count--;
        // if (0 == g_read_count){
         if (1 == g_read_count){    
            printf("reader unlock\n");
            pthread_mutex_unlock(&g_lock);
        }
        printf("reader g_lock_read unlock 2\n");
        pthread_mutex_unlock(&g_lock_read);

        sleep(5);
    }
}

void *proc_writer(void *lparam)
{
    unsigned int tid =pthread_self();
    while (1) {
        pthread_mutex_lock(&g_lock);
        g_buf = to_string(tid) + " xxxxxxxxxxxxxxxxxxx";
        printf ("[%u] write:%s\n", tid, g_buf.c_str());
        pthread_mutex_unlock(&g_lock);
        sleep(10);
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