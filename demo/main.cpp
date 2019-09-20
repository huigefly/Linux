#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

const int MAX_PTHREAD = 10;

void *write_proc(void *lparam)
{
    FILE *fp = fopen ("./all_fp", "w+");
    printf ("tid:%d, %p\n", pthread_self(), fp);
    int i=0;
    while (1)
    {
        char szBuf[1024] = {0};
        sprintf (szBuf, "tid:%d,context:helloworld_%d\n", pthread_self(), i++);
        fprintf (fp, "%s", szBuf);
        sleep (0.5);
    }

    pthread_exit (0);
}

int f(int, float)
{
	return 0;
}
int main()
{
    pthread_t tid[MAX_PTHREAD];
    
    for (int i=0; i<MAX_PTHREAD; i++) {
        pthread_create(&tid[i], NULL, write_proc, NULL);
    }

    for (int i=0; i<MAX_PTHREAD; i++) {
        pthread_join(tid[i], NULL);
    }

    return 0;
}
