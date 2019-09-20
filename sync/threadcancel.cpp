#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
 
pthread_t g_tid;

void first_cleanup(void *arg)
{
    printf("this is clenup function %s\n", arg);
}
 
void second_cleanup(void *arg) 
{
    printf("this is clenup function %s\n", arg);
}
 
void *thread_fun1(void *arg)
{
    sleep(1);
    printf("this is thread 1\n");
    char *pszBuf = (char *)malloc (1024);
    pthread_cleanup_push(first_cleanup, (void *)"thread 1");
    pthread_cleanup_push(second_cleanup, (void *)"thread 1");   
     
    while (1) {
        printf ("this is thread 1 sleeping 1\n");
        //pthread_testcancel();
        sleep (5);
        printf ("this is thread 1 sleeping 1 ok cancel pointer\n");
        pthread_testcancel();
        printf ("this is thread 1 sleeping 2\n");
        sleep (10);
        pthread_testcancel();
    }

    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);    
    return (void*)0;
 } 
 
void *thread_fun2(void *arg)
{       
    sleep(1);
    printf("this is thread 2\n");
    char *pszBuf = (char *)malloc (1024);
    pthread_cleanup_push(first_cleanup, (void *)"thread 2");
    pthread_cleanup_push(second_cleanup, (void *)"thread 2");
    
    sleep (3);
    printf ("thread_fun2 send cancel\n");
    int n = pthread_cancel(g_tid);
    printf ("thread 2 n:%d\n", n);
    
    pthread_exit((void*)0);
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);       
 } 
 
void *thread_fun3(void *arg)
{
    sleep(1);
    printf("this is thread 3\n");
    char *pszBuf = (char *)malloc (1024);
    pthread_cleanup_push(first_cleanup, (void *)"thread 3");
    pthread_cleanup_push(second_cleanup, (void *)"thread 3");
    
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);     
    pthread_exit((void*)0);  
 } 
 
 int main()
 {
     pthread_t tid1, tid2, tid3;
     pthread_create(&tid1, NULL, thread_fun1, NULL);
     pthread_create(&tid2, NULL, thread_fun2, NULL);
     pthread_create(&tid3, NULL, thread_fun3, NULL);
     g_tid = tid1;

     pthread_join(tid1, NULL);
     pthread_join(tid2, NULL);
     pthread_join(tid3, NULL);

     sleep (20);
     return 0;
}