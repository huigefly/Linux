/*
全双工经典写法：
    父进程往子进程写，子进程也可以往父进程写
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int fdpc[2], fdcp[2];
    pipe (fdpc);
    pipe (fdcp);

    pid_t pid = fork ();
    if (0 == pid) {
        close (fdpc[1]);
        close (fdcp[0]);
        char szBuf[16];
        printf ("child read wait:\n");
        int n = read (fdpc[0], szBuf, 16);
        szBuf[n] = '\0';
        printf ("clild recv buf:%s\n", szBuf);

        write (fdcp[1], "i am child", 10);
        exit(0);
    }

    close (fdpc[0]);
    close (fdcp[1]);
    sleep (1);
    write (fdpc[1], "helloworld", 10);
    char buf[16];
    read(fdcp[0], buf, 16);
    printf ("parent recv buf:%s\n", buf);

    waitpid (pid, NULL, 0);

    return 0;
}