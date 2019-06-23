#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "libyy_socket/yySocket.h"

#define PORT 6666

void wait_pid(int signo)
{
    while (waitpid (0, NULL, WNOHANG) > 0);
    return ;
}

int main()
{
    int sfd = Socket (AF_INET, SOCK_STREAM, 0);
    int cfd;
    struct sockaddr_in addr_ser, addr_cli;
    bzero (&addr_cli, sizeof (addr_cli));
    bzero (&addr_ser, sizeof (addr_ser));

    addr_ser.sin_family = AF_INET;
    addr_ser.sin_port = htons (PORT); 
    addr_ser.sin_addr.s_addr = htonl (INADDR_ANY);

    socklen_t nLen = sizeof (addr_ser);
    Bind (sfd, (struct sockaddr *)&addr_ser, nLen);
    Listen (sfd, 128);

    while (1) {
        socklen_t nCliLen = sizeof (addr_cli); 
        printf ("ser accept waiting.....\n");
        cfd = Accept (sfd, (struct sockaddr *)&addr_cli, &nCliLen);
        char szCliIp[16] = {0};
        printf ("clinet ip:%s, port:%d\n", inet_ntop (AF_INET, &addr_cli.sin_addr.s_addr, szCliIp, sizeof (szCliIp)),
                ntohs (addr_cli.sin_port));    

        pid_t pid = fork ();
        if (pid < 0) {
            perror ("fork error");
        } else if(pid == 0) {
            close (sfd);
            break;
        } else {
            // parent
            close (cfd);
            signal (SIGCHLD, wait_pid);
        }
    }

    int nCount = 3;
    while (nCount--) {
        char szBuf[BUFSIZ] = {0};
        int nLen = Read (cfd, szBuf, BUFSIZ);
        Write (STDOUT_FILENO, szBuf, nLen);
        int i, nSize = strlen (szBuf);
        for (i=0; i<nSize; i++)
            szBuf[i] =  toupper (szBuf[i]);
        Write (cfd, szBuf, nLen);
    }

    Close (cfd);

    return 0;
}