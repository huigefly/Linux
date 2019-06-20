#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

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
    int sfd = socket (AF_INET, SOCK_STREAM, 0);
    if (-1 == sfd) {
        exit (-2);
    }
    socklen_t nLen;
    nLen = sizeof (nLen);
    struct sockaddr_in addr;
    memset (&addr, 0, sizeof (addr));    
    addr.sin_family = AF_INET;
    addr.sin_port = htons (nPort);
    inet_pton (AF_INET,(const char *)szIp, &addr.sin_addr.s_addr);
    if (-1 == connect (sfd, (struct sockaddr *)&addr, sizeof (addr))) {
        printf ("connect error");
        return (-3);
    }
    while (1) {
        char szBuf[BUFSIZ] = {0};
        fgets (szBuf, sizeof (szBuf), stdin);
        write (sfd, szBuf, strlen (szBuf));
        int value = read (sfd, szBuf, sizeof (szBuf));
        //printf ("recv:%s", szBuf);
        write (STDOUT_FILENO, szBuf, value);
    }

    close (sfd);

    return 0;
}
