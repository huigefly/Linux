#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define PORT 6666
#define MAX_EVENTS 1024
#define BUF_SIZE 4096

typedef enum
{
    ENUM_TREE_INVALID = -1,
    ENUM_TREE_IN,
    ENUM_TREE_UNDER,
    ENUM_TREE_MAX
} ENUM_TREE_T;

typedef void (*CALL_BACK_T)(int, int, void *);

typedef struct
{
    int fd;                //要监听的文件描述符
    int events;            //对应的监听事件
    void *arg;             //泛型参数
    CALL_BACK_T call_back; //回调函数
    ENUM_TREE_T status;    //是否在监听:1->在红黑树上(监听), 0->不在(不监听)
    char buf[BUF_SIZE];
    int len;
    long last_active; //记录每次加入红黑树 g_efd 的时间值
} EVENT_T;

int g_efd;
EVENT_T g_events[MAX_EVENTS + 1];

void recvdata(int fd, int events, void *arg);

// 针对这个返回的句柄做初始化
void eventset(EVENT_T *pEv, int fd, CALL_BACK_T call_back, void *arg)
{
    pEv->fd = fd;
    pEv->events = 0;
    pEv->arg = arg;
    pEv->call_back = call_back;
    pEv->status = ENUM_TREE_UNDER;
    pEv->last_active = time(NULL);
}

// 将自定义的event，添加到树上
void eventadd(int efd, int events, EVENT_T *pEv)
{
    int op;
    struct epoll_event evt = {0, {0}};

    evt.events = events;
    evt.data.ptr = pEv;

    pEv->events = events;
    if (pEv->status == ENUM_TREE_IN)
    {
        op = EPOLL_CTL_MOD;
    }
    else
    {
        op = EPOLL_CTL_ADD;
        pEv->status = ENUM_TREE_IN;
    }

    if (epoll_ctl(efd, op, pEv->fd, &evt) < 0) //实际添加/修改
        printf("event add failed [fd=%d], events[%d]\n", pEv->fd, events);
    else
        printf("event add OK [fd=%d], op=%d, events[%0X]\n", pEv->fd, op, events);

    return;
}

// 自定义的event从树上删除
void eventdel(int efd, EVENT_T *pEv)
{
    struct epoll_event epv = {0, {0}};
    if (pEv->status == ENUM_TREE_UNDER) //不在红黑树上
        return;

    epv.data.ptr = pEv;
    pEv->status = ENUM_TREE_UNDER;                //修改状态
    epoll_ctl(efd, EPOLL_CTL_DEL, pEv->fd, &epv); //从红黑树 efd 上将 ev->fd 摘除

    return;
}

void senddata(int fd, int events, void *arg)
{
    EVENT_T *pEv = (EVENT_T *)arg;
    int len;

    len = send(fd, pEv->buf, pEv->len, 0); //直接将数据 回写给客户端。未作处理
    /*
    printf("fd=%d\tev->buf=%s\ttev->len=%d\n", fd, pEv->buf, pEv->len);
    printf("send len = %d\n", len);
    */

    if (len > 0)
    {
        printf("send[fd=%d], [%d]%s\n", fd, len, pEv->buf);
        eventdel(g_efd, pEv);             //从红黑树g_efd中移除
        eventset(pEv, fd, recvdata, pEv); //将该fd的 回调函数改为 recvdata
        eventadd(g_efd, EPOLLIN, pEv);    //从新添加到红黑树上， 设为监听读事件
    }
    else
    {
        close(pEv->fd);       //关闭链接
        eventdel(g_efd, pEv); //从红黑树g_efd中移除
        printf("send[fd=%d] error %s\n", fd, strerror(errno));
    }

    return;
}

void recvdata(int fd, int events, void *arg)
{
    EVENT_T *pEv = (EVENT_T *)arg;
    int len;
    len = recv(fd, pEv->buf, sizeof(pEv->buf), 0); //读文件描述符, 数据存入myevent_s成员buf中

    eventdel(g_efd, pEv); //将该节点从红黑树上摘除

    if (len > 0)
    {
        pEv->len = len;
        pEv->buf[len] = '\0'; //手动添加字符串结束标记
        printf("C[%d]:%s\n", fd, pEv->buf);

        eventset(pEv, fd, senddata, pEv); //设置该 fd 对应的回调函数为 senddata
        eventadd(g_efd, EPOLLOUT, pEv);   //将fd加入红黑树g_efd中,监听其写事件
    }
    else if (len == 0)
    {
        close(pEv->fd);
        /* ev-g_events 地址相减得到偏移元素位置 */
        printf("[fd=%d] pos[%ld], closed\n", fd, pEv - g_events);
    }
    else
    {
        close(pEv->fd);
        printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
    }

    return;
}

void acceptconn(int lfd, int events, void *arg)
{
    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    socklen_t len = sizeof(cliaddr);
    int cfd = accept(lfd, (struct sockaddr *)&cliaddr, &len);
    if (-1 == cfd)
    {
        if (errno != EAGAIN && errno != EINTR)
        {
            /* 暂时不做出错处理 */
        }
        printf("%s: accept, %s\n", __func__, strerror(errno));
        return;
    }

    do
    {
        int i;
        // 要把cfd挂到树上，先将cfd代入自己的event，所以先找到自己的event位置
        for (i = 0; i < MAX_EVENTS; i++)
        {
            if (g_events[i].status == ENUM_TREE_UNDER)
            {
                break;
            }
        }

        // 如果树上已经挂满了，无法设置到树上
        if (i == MAX_EVENTS)
        {
            printf("%s: max connect limit[%d]\n", __func__, MAX_EVENTS);
            break; //跳出do while(0) 不执行后续代码
        }

        // 设置cfd 为非阻塞方式
        int flag = 0;
        if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0)
        {
            printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
            break;
        }

        // 设置到树上
        eventset(&g_events[i], cfd, recvdata, &g_events[i]);
        eventadd(g_efd, EPOLLIN, &g_events[i]);
    } while (0);
}

/*创建 socket, 初始化lfd */
void initListenSocket(int efd, short port)
{
    int i;
    for (i = 0; i <= MAX_EVENTS; i++)
    {
        g_events[i].status = ENUM_TREE_UNDER;
    }

    // 创建socket
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(lfd, F_SETFL, O_NONBLOCK); //listen fd 也要是非阻塞的将socket设为非阻塞

    // 设置fd 和回调函数等的绑定
    /* void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *), void *arg);  */
    eventset(&g_events[MAX_EVENTS], lfd, acceptconn, &g_events[MAX_EVENTS]);

    /* void eventadd(int efd, int events, struct myevent_s *ev) */
    eventadd(efd, EPOLLIN, &g_events[MAX_EVENTS]);

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin)); //bzero(&sin, sizeof(sin))
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    bind(lfd, (struct sockaddr *)&sin, sizeof(sin));

    listen(lfd, 20);

    return;
}

int main()
{
    // +1: listen fd
    g_efd = epoll_create(MAX_EVENTS + 1);
    if (-1 == g_efd)
    {
        return -1;
    }

    initListenSocket(g_efd, PORT);

    struct epoll_event events[MAX_EVENTS];
    printf("server running:port[%d]\n", PORT);
    int checkpos = 0, i;
    while (1)
    {
        /* 超时验证，每次测试100个链接，不测试listenfd 当客户端60秒内没有和服务器通信，则关闭此客户端链接 */
        long now = time(NULL); //当前时间
        for (i = 0; i < 100; i++, checkpos++)
        { //一次循环检测100个。 使用checkpos控制检测对象
            if (checkpos == MAX_EVENTS)
                checkpos = 0;
            if (g_events[checkpos].status != ENUM_TREE_IN) //不在红黑树 g_efd 上
                continue;

            long duration = now - g_events[checkpos].last_active; //客户端不活跃的世间
            if (duration >= 6)
            {
                close(g_events[checkpos].fd); //关闭与该客户端链接
                printf("[fd=%d] timeout\n", g_events[checkpos].fd);
                eventdel(g_efd, &g_events[checkpos]); //将该客户端 从红黑树 g_efd移除
            }
        }

        int nRet = epoll_wait(g_efd, events, MAX_EVENTS, 1000);
        if (-1 == nRet)
        {
            printf("epoll_wait error, exit\n");
            break;
        }

        for (i = 0; i < nRet; i++)
        {
            EVENT_T *pEv = (EVENT_T *)events[i].data.ptr;
            if ((events[i].events) & EPOLLIN && (pEv->events & EPOLLIN))
            {
                pEv->call_back(pEv->fd, EPOLLIN, pEv);
            }
            else if ((events[i].events & EPOLLOUT) && (pEv->events & EPOLLOUT))
            {
                pEv->call_back(pEv->fd, EPOLLOUT, pEv);
            }
        }
    }

    return 0;
}
