#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include "libyy_socket/yySocket.h"

#define PORT 6666
#define OPEN_MAX 5000

void cb_epoll(int fd)
{
    printf ("func:%s, param fd:%d\n", __FUNCTION__, fd);
}

typedef void (*callback)(int fd);

typedef struct{
    int fd;
    callback cb;
}DE_PARAM_T;

int main()
{
    int sfd = Socket (AF_INET, SOCK_STREAM, 0);
    // 端口复用
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr_ser, addr_cli;
    bzero (&addr_cli, sizeof (addr_cli));
    bzero (&addr_ser, sizeof (addr_ser));

    addr_ser.sin_family = AF_INET;
    addr_ser.sin_port = htons (PORT); 
    addr_ser.sin_addr.s_addr = htonl (INADDR_ANY);

    socklen_t nLen = sizeof (addr_ser);
    Bind (sfd, (struct sockaddr *)&addr_ser, nLen);
    Listen (sfd, 128);

    

    int efd = epoll_create (OPEN_MAX);
    struct epoll_event evt;
    //evt.events = EPOLLIN;//默认开启水平出发模式
    evt.events = EPOLLIN | EPOLLET;  // 边缘出发：有client发送才有响应
    evt.data.fd = sfd;
    epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &evt);

    while (1) {
        struct epoll_event oevt[OPEN_MAX];
        int nRet = epoll_wait (efd, oevt, OPEN_MAX, -1);
        if (nRet == -1) {
            perr_exit ("epoll_wait error");
        }
        int i;
        for (i=0; i<nRet; i++) {
            if (!(oevt[i].events & EPOLLIN)) {
                continue;
            }

            // sfd可读，说明有客户端发送请求过来，可以accept
            if (oevt[i].data.fd == sfd) {
                struct sockaddr_in cliaddr;
                memset (&cliaddr, 0, sizeof (cliaddr));
                char str[64] = {0};
                socklen_t nLen = sizeof (cliaddr); 
                int cfd = Accept (sfd, (struct sockaddr*)&cliaddr, &nLen);
                printf("received from %s at PORT %d\n", 
                        inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), 
                        ntohs(cliaddr.sin_port));
                
                // 设置句柄非阻塞模式
                int flag = fcntl(cfd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl (cfd, F_SETFL, flag);

                // 自定义结构带入参数，union结构
                
                DE_PARAM_T param;
                param.cb = cb_epoll;
                param.fd = cfd;
                
                // evt.events = EPOLLIN; // 前面写了，这边不用多些一次
                evt.data.ptr = &param;

                nRet = epoll_ctl (efd, EPOLL_CTL_ADD, cfd, &evt);
                if (-1 == nRet) {
                    perr_exit ("epoll_ctl error");
                }
            } else {
                char szBuf[10] = {0};
                DE_PARAM_T *param = oevt[i].data.ptr;
                int cfd = param->fd;
                printf ("Ready read.\n");
                (*param->cb)(cfd);
                while ((nRet = Read (cfd, szBuf, sizeof (szBuf))) >= 0 ) {
                    if (0 == nRet) {
                        epoll_ctl (efd, EPOLL_CTL_DEL, cfd, NULL);
                        Close (cfd);
                        continue;
                    } else if (nRet < 0) {
                        epoll_ctl (efd, EPOLL_CTL_DEL, cfd, NULL);
                        Close (cfd);
                    } else {
                        int nBufi;
                        for (nBufi = 0; nBufi < nRet; nBufi++)
                            szBuf[nBufi] = toupper(szBuf[nBufi]);   //转大写,写回给客户端
                        Write(STDOUT_FILENO, szBuf, nRet);
                        Writen(cfd, szBuf, nRet);
                    }
                }
                printf ("Ready read ok.\n");
            }
        }
    }

    Close (sfd);
    Close (efd);

    return 0;
}