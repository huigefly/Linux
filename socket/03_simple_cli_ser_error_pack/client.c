#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "libyy_socket/yySocket.h"

int sfd;

void handle_sig()
{
  printf ("func line:%d\n", __LINE__);
  if (-1 != sfd) {
    Close (sfd);
    sfd = -1;
  }
  exit (0);
}

void signalQuit (void)
{
  struct sigaction act;
  memset (&act, 0, sizeof (act));
  act.sa_handler = handle_sig;
  sigemptyset (&act.sa_mask);
  act.sa_flags = 0;
  sigaction (SIGTERM, &act, 0);
  sigaction (SIGQUIT, &act, 0);
  sigaction (SIGINT, &act, 0);
}

int main(int argc, char *argv[])
{
    signalQuit();
    char *pszIp = argv[1];
    char szIp[BUFSIZ] = {0};
    memcpy (szIp, pszIp, strlen (pszIp));
    int nPort = atoi (argv[2]);
    printf ("ip:%s,port:%d\n", pszIp, nPort);
    if (nPort < 1000)
    {   
        exit (-1);
    }
    sfd = Socket (AF_INET, SOCK_STREAM, 0);
    socklen_t nLen;
    nLen = sizeof (nLen);
    struct sockaddr_in addr;
    memset (&addr, 0, sizeof (addr));    
    addr.sin_family = AF_INET;
    addr.sin_port = htons (nPort);
    inet_pton (AF_INET,(const char *)szIp, &addr.sin_addr.s_addr);
    Connect (sfd, (struct sockaddr *)&addr, sizeof (addr));
    int nCount = 1;
    while (1) {
        char szBuf[BUFSIZ] = {0};
        sprintf (szBuf, "%d--count:%d\n", getpid (), nCount++);
//        fgets (szBuf, sizeof (szBuf), stdin);
        Write (sfd, szBuf, strlen (szBuf));
        int value = Read (sfd, szBuf, sizeof (szBuf));
        //printf ("recv:%d\n", value);
        if (value == 0)
        {
            Close (sfd);
            return 0;
        }
        Write (STDOUT_FILENO, szBuf, value);
        sleep (0.5);
    }
    printf ("func line:%d\n", __LINE__);
    Close (sfd);

    return 0;
}
