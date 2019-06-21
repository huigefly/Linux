#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include "libyy_socket/yySocket.h"

int main(int argc, char *argv[])
{
    char *pszIp = argv[1];
    char szIp[BUFSIZ] = {0};
    memcpy (szIp, pszIp, strlen (pszIp));
    int nPort = atoi (argv[2]);
    printf ("ip:%s,port:%d\n", pszIp, nPort);
    if (nPort < 1000)
    {   
        exit (-1);
    }
    int sfd = Socket (AF_INET, SOCK_STREAM, 0);
    socklen_t nLen;
    nLen = sizeof (nLen);
    struct sockaddr_in addr;
    memset (&addr, 0, sizeof (addr));    
    addr.sin_family = AF_INET;
    addr.sin_port = htons (nPort);
    inet_pton (AF_INET,(const char *)szIp, &addr.sin_addr.s_addr);
    Connect (sfd, (struct sockaddr *)&addr, sizeof (addr));
    while (1) {
        char szBuf[BUFSIZ] = {0};
        fgets (szBuf, sizeof (szBuf), stdin);
        Write (sfd, szBuf, strlen (szBuf));
        int value = Read (sfd, szBuf, sizeof (szBuf));
        //printf ("recv:%s", szBuf);
        Write (STDOUT_FILENO, szBuf, value);
    }

    Close (sfd);

    return 0;
}
