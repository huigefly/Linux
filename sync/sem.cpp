#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
using namespace std;

deque<string> g_deque;
sem_t *g_semget;
sem_t *g_semput;

void *procPro(void *lparam)
{
    while (1){
        
        char szBuf[1024] = {0};
        sprintf (szBuf, "hellowordl_%d", rand());
        printf ("proc pro wait\n");
        //pthread_mutex_lock (&mtxput);
        sem_wait (g_semput);
        printf ("proc pro wait end\n");
        g_deque.push_back (szBuf);
        sleep (1);
        sem_post (g_semget);
        //pthread_mutex_unlock (&mtxget);
    }
}

void *procConsumer(void *lparam)
{
    while (1){
        printf ("procConsumer lock\n");
       // pthread_mutex_lock (&mtxget);
        sem_wait (g_semget);
        if (g_deque.size () > 0) {
            g_deque.pop_front();
            cout << "procconsumer: " << g_deque.size () << endl;
        }
        sleep (5);
        sem_post (g_semput);
       // pthread_mutex_unlock (&mtxput);
    }
}

int main()
{
    g_semget = sem_open ("sem_getaaa", O_CREAT, 0644, 1);
    g_semput = sem_open ("sem_putaaa", O_CREAT, 0644, 1);

    pthread_t tidPro[1];
    for (int i=0; i<1; i++) {
        pthread_create(&tidPro[i], NULL, procPro, NULL);
    }

    pthread_t tidCon[10];
    for (int i=0; i<1; i++) {
        pthread_create(&tidCon[i], NULL, procConsumer, NULL);
    }

    for (int i=0; i<3; i++) {
        pthread_join (tidPro[i], NULL);
    }
    

    return 0;
}