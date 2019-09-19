#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <pthread.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <semaphore.h>
using namespace std;

sem_t g_sem;
vector<string> g_vec;

void *proc(void *lparam)
{
    int flag = 0;
    while (1)
    {
        if (0 == flag){
            cout << "sem_wait" << endl;
            if (0 == sem_wait (&g_sem)) {
                int size = g_vec.size();
                g_vec[size -1] = "nihao-1";
                flag = 1;
            }
            cout << "sem_wait end" << endl;
        }else if (1 == flag){
            cout << "sem_trywait donging" << endl;
            if (0 == sem_trywait (&g_sem)) {
                cout << "trywaiting " << endl;
                sleep (1);
                flag = 0;
                continue;
            }
            string s = g_vec.back();
            g_vec.pop_back();
            cout << "try:" << s << endl;
            sleep (1);
            cout << "sem_trywait end" << endl;
        }
    }
}

void *proco(void *lparam)
{
    while (1)
    {
        sleep (5);
        char szBuf[1024] = {0};
        sprintf (szBuf, "helloworld_%d", rand());
        g_vec.push_back (szBuf);
        sem_post (&g_sem);
    }
}

int main()
{
    sem_init (&g_sem, 0, 0);

    for (int i=0; i<100; i++){
        char szBuf[1024] = {0};
        sprintf (szBuf, "helloworld_%d", i);
        g_vec.push_back (szBuf);
    }

    pthread_t tid;
    pthread_create (&tid, NULL, proc, NULL);

    pthread_t tido;
    pthread_create (&tido, NULL, proco, NULL);

    sem_destroy(&g_sem);

    sleep (1000);

    return 0;
}