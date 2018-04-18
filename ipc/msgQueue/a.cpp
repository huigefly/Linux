#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/msg.h>  
#include <fcntl.h>  
  
#define PATH_NAME 	"./AQueueB"  
#define MAXLINE 	1024

typedef struct
{  
    long msgtype;  
    char msgtext[MAXLINE];  
}MSG_BUF_T;  
  
key_t MakeKey(const char *pathname)
{
	int fd;  
	if((fd = open(pathname, O_CREAT, 0666)) < 0)  
	{
		printf ("open error");
	}
    close(fd);  
    return ftok(pathname, 880);  
}
 
void *onProcSend(void *lParam)
{
	int msgid = *(int *)lParam;
	printf("pthread send msgid:%d\n", msgid);
	int i = 0;
	while (1)
    {
    	char str1[] = "AAAA test message: Wellcome.";
    	MSG_BUF_T sndmsg;
    	memset (&sndmsg, 0, sizeof(sndmsg));
    	
    	sndmsg.msgtype = 200;  
	    sprintf(sndmsg.msgtext, "%s", str1);  
	    if(msgsnd(msgid, (struct msgbuf *)&sndmsg, sizeof(sndmsg.msgtext)+1, IPC_NOWAIT) == -1)  
	    {
	    	printf ("msgsnd error\n\n");
	    	// break;
	    }
	    printf("send ok, buf:%s, msgid:%d\n", sndmsg.msgtext, msgid);
	    sleep(10);
    }

	pthread_exit((void *)0);
}

void *onProcRecv(void *lParam)
{
	int msgid = *(int *)lParam;
	printf("pthread recv msgid:%d\n", msgid);
	char str2[] = "AAAA test message: goodbye.";  
    MSG_BUF_T rcvmsg;
	while (1)
	{
    	memset (&rcvmsg, 0, sizeof(rcvmsg));
		if(msgrcv(msgid, (struct msgbuf*)&rcvmsg, sizeof(rcvmsg), 100, MSG_NOERROR) == -1)  
	    {
	    	printf ("msgrcv error\n");
	    	//pthread_exit((void *)0);
	    	sleep(5);
	    }
	    printf("Recevied message:\n%s\n", rcvmsg.msgtext);  
	}
	pthread_exit((void *)0);
}
 
int main(void)  
{  
    int msgid;  
    key_t key;  
    key = MakeKey(PATH_NAME);  
    
    if((msgid = msgget(4446, IPC_CREAT | 0666)) == -1)  
    {
    	printf ("msgget error");
    }
    
    // create send pthread
    printf(" -- msgid:%d\n", msgid);
    pthread_t tidSend = -1;
    if (-1 == pthread_create(&tidSend, NULL, onProcSend, (void *)&msgid))
    {
    	printf("pthread create tidSend failed!\n");
    }
    
    // createa recv pthread
    pthread_t tidRecv = -1;
    if (-1 == pthread_create(&tidRecv, NULL, onProcRecv, (void *)&msgid))
    {
    	printf("pthread create tidRecv failed!\n");
    }
    
    
    
    pthread_join(tidSend, NULL);
    pthread_join(tidRecv, NULL);
    
    msgctl(msgid, IPC_RMID, 0);
    exit(0);  
}  
