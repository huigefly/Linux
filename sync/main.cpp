#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
using namespace std;

deque<string> g_deque;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *procPro(void *lparam)
{
    while (1){
        sleep (1);
        char szBuf[1024] = {0};
        pthread_mutex_lock (&mutex);
        sprintf (szBuf, "hellowordl_%d", rand());
        g_deque.push_back (szBuf);
        pthread_mutex_unlock (&mutex);
        pthread_cond_broadcast (&cond);
    }
}

void *procConsumer(void *lparam)
{
    while (1){
        pthread_mutex_lock (&mutex);
        while (g_deque.size () == 0){
            //printf ("proc consumer id:%u, waiting\n", pthread_self ());
            pthread_cond_wait (&cond, &mutex);
        }
        string s = g_deque.front();
        g_deque.pop_front();
        pthread_mutex_unlock (&mutex);
        cout << "procconsumer:" << s << ", tid:" << pthread_self () << endl;
    }
}

int main()
{
    pthread_t tidPro[3];
    for (int i=0; i<3; i++) {
        pthread_create(&tidPro[i], NULL, procPro, NULL);
    }

    pthread_t tidCon[2];
    for (int i=0; i<2; i++) {
        pthread_create(&tidCon[i], NULL, procConsumer, NULL);
    }

    for (int i=0; i<3; i++) {
        pthread_join (tidPro[i], NULL);
    }
    

    return 0;
}