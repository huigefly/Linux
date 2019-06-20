#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 6868

int main()
{
    int i;
    int sfd = socket (AF_INET, SOCK_STREAM, 0);
    if (-1 == sfd) {
        return -1;
    }

    struct sockaddr_in addr;
    memset (&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl (INADDR_ANY);
    addr.sin_port = htons (PORT);
    if (-1 == bind (sfd, (struct sockaddr *)&addr, sizeof (addr))) {
        return -2;
    }

    if (-1 == listen (sfd, 128)) {
        return -3;
    }

    socklen_t nLen;
    nLen = sizeof (nLen);
    struct sockaddr_in addr_cli;
    memset (&addr_cli, 0, sizeof (addr_cli));
    int cfd = accept (sfd, (struct sockaddr*)&addr_cli, &nLen);
    if (-1 == cfd) {
        return -4;
    }

    // 简单地将接受的字符串改为大写，返回。
    // 使用cfd 读写
    while (1) {
        char szBuf[BUFSIZ] = {0};
        int nReadLen = read (cfd, szBuf, sizeof (szBuf));
        write (STDOUT_FILENO, szBuf, nReadLen);

        for (i=0; i<nReadLen; i++) {
            szBuf[i] = toupper (szBuf[i]);
        }

       write (cfd, szBuf, nReadLen);
    }

    close (sfd);
    close (cfd);

    return 0;
}
