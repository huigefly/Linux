/*
半双工经典写法：
    父进程往子进程写，子进程不能往父进程写?
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int fd[2];
    pipe (fd);
    pid_t pid = fork ();
    if (0 == pid) {
        close (fd[1]);
        char szBuf[16];
        printf ("child read wait:\n");
        int n = read (fd[0], szBuf, 16);
        szBuf[n] = '\0';
        printf ("buf:%s\n", szBuf);
        exit(0);
    }

    close (fd[0]);
    sleep (1);
    write (fd[1], "helloworld", 10);

    waitpid (pid, NULL, 0);

    return 0;
}