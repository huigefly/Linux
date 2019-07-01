#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

pthread_mutex_t lock;
pthread_cond_t cond;

void *proc(void *lparam)
{
    while (1){
        printf ("i am wait condition\n");
        //pthread_cond_wait (&cond, &lock);
        pthread_cond_wait (&cond, NULL);
        printf ("tid:%u\n", pthread_self ());
        sleep (5);
    } 


    pthread_exit (0);
}

void *adproc(void *lparam)
{
    while (1) {
        printf ("adjust signal send\n");
        //pthread_cond_signal (&cond);
        pthread_cond_broadcast (&cond);
        printf ("adjust signal sned ok\n");
        sleep (10);
    }
}

int main()
{

    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_mutex_init (&lock, NULL);
    pthread_cond_init (&cond, NULL);

    pthread_t *tid = (pthread_t *)malloc (sizeof (pthread_t) * 10);

    int i;
    for (i=0; i<10; i++) {
        pthread_create (&tid[i], NULL, proc, NULL);
    }

    pthread_t adjust;
    pthread_create (&adjust, NULL, adproc, NULL);


    while (1) sleep (10);


    return 0;
}
