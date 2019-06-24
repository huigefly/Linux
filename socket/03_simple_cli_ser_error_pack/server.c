#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include "libyy_socket/yySocket.h"

#define PORT 6868

int main()
{
    int i;
    int sfd = Socket (AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    memset (&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl (INADDR_ANY);
    addr.sin_port = htons (PORT);

    Bind (sfd, (struct sockaddr *)&addr, sizeof (addr));

    Listen (sfd, 128);

    socklen_t nLen;
    nLen = sizeof (nLen);
    struct sockaddr_in addr_cli;
    memset (&addr_cli, 0, sizeof (addr_cli));
    int cfd = Accept (sfd, (struct sockaddr*)&addr_cli, &nLen);
    
    // echo client info
    char szIp[BUFSIZ] = {0};
    printf ("client ip:%s, port:%d\n", 
        inet_ntop (AF_INET, &addr_cli.sin_addr.s_addr, szIp, sizeof (szIp)), 
         ntohs (addr_cli.sin_port));

    // 简单地将接受的字符串改为大写，返回。
    // 使用cfd 读写
    while (1) {
        char szBuf[BUFSIZ] = {0};
        int nReadLen = Read (cfd, szBuf, sizeof (szBuf));
        if (0 == nReadLen) {
		printf ("client exit, server follow\n");
		break;
	}
	Write (STDOUT_FILENO, szBuf, nReadLen);

        for (i=0; i<nReadLen; i++) {
            szBuf[i] = toupper (szBuf[i]);
        }

       Write (cfd, szBuf, nReadLen);
    }

    Close (sfd);
    Close (cfd);

    return 0;
}
