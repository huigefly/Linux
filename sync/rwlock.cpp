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
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

void *procPro(void *lparam)
{
    while (1){
        sleep (1);
        char szBuf[1024] = {0};
        sprintf (szBuf, "hellowordl_%d", rand());
        pthread_rwlock_wrlock (&rwlock);
        g_deque.push_back (szBuf);
        pthread_rwlock_unlock (&rwlock);
    }
}

void *procConsumer(void *lparam)
{
    while (1){
        sleep (1);
        pthread_rwlock_rdlock (&rwlock);
        if (g_deque.size () > 0) {
            string s = g_deque.back();
            cout << "procconsumer:" << s << endl;
        }
        pthread_rwlock_unlock (&rwlock);
    }
}

int main()
{
    pthread_t tidPro[1];
    for (int i=0; i<1; i++) {
        pthread_create(&tidPro[i], NULL, procPro, NULL);
    }

    pthread_t tidCon[10];
    for (int i=0; i<10; i++) {
        pthread_create(&tidCon[i], NULL, procConsumer, NULL);
    }

    for (int i=0; i<3; i++) {
        pthread_join (tidPro[i], NULL);
    }
    

    return 0;
}