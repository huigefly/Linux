#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define SIZE 100
#define MALLOC_SIZE (1024*1024)
int main()
{
    int a=0;
    printf("%d\n", getpid());
    pid_t t = fork();
    if (t!=0){
        printf("pid:%d, %d, %p\n", getpid(), a, &a);
    } else {
        a = 10;
        printf("pid:%d, %d, %p\n", getpid(),a, &a);
    }

    return 0;
}
