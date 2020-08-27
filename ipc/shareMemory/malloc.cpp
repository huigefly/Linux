#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SIZE 100
#define MALLOC_SIZE (1024*1024)
int main()
{
    char *pszBuffer[SIZE];
    int i = 0;
    for (; i < SIZE; i++) {
        pszBuffer[i] = (char *)malloc(MALLOC_SIZE);
        sprintf(pszBuffer[i], "%d helloworld", i);
        // for (int k=0; k < MALLOC_SIZE; i++){

        // }
    }

    for (i = 0; i < SIZE; i++) {
        printf("%s\n", pszBuffer[i]);
    }

    sleep(1000);

    return 0;
}
