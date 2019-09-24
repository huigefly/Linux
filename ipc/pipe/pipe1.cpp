/*
单个进程中使用pipe
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int g_fd[2];
int i;

void *rproc(void *lparam)
{
    while (1){
        char buf[1024];
        read (g_fd[0], buf, 1024);
        printf ("pid:%d, buf:%s\n", getpid(), buf);
        sleep (1);
    }
}

void *wproc(void *lparam)
{
    while (1){
        char buf[1024];
        i++;
        sprintf (buf, "helloworld_%d", i);
        write (g_fd[1], buf, 1024);
        sleep (1);
    }
}

int main()
{
    pipe (g_fd);

    pthread_t tidr;
    pthread_t tidw;

    pthread_create (&tidr, NULL, rproc, NULL);
    pthread_create (&tidr, NULL, wproc, NULL);

    pthread_join (tidr, NULL);

    return 0;
}