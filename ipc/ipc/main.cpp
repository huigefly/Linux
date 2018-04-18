#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>  
#include <fcntl.h>  
  
#define PATH_NAME 	"./Queue"  
#define MAXLINE 	1024
  
key_t MakeKey(const char *pathname)
{
	int fd;  
	if((fd = open(pathname, O_CREAT, 0666)) < 0)  
	{
		printf ("open error");
	}
    close(fd);  
    return ftok(pathname, 0);  
}
  
int main(void)  
{  
    int msgid;  
    int status;  
    key_t key;  
    key = MakeKey(PATH_NAME);  
  
    char str1[] = "test message: Wellcome.";  
    char str2[] = "test message: goodbye.";  
    struct msgbuf  
    {  
        long msgtype;  
        char msgtext[MAXLINE];  
    }sndmsg, rcvmsg;  
  
    if((msgid = msgget(key, IPC_CREAT | 0666)) == -1)  
    {
    	printf ("msgget error");
    }
    
    sndmsg.msgtype = 100;  
    sprintf(sndmsg.msgtext, "%s", str1);  
    if(msgsnd(msgid, (struct msgbuf *)&sndmsg, sizeof(str1)+1, 0) == -1)  
    {
    	printf ("msgsnd error");
    }
    sndmsg.msgtype = 200;  
    sprintf(sndmsg.msgtext, "%s", str2);  
    if(msgsnd(msgid, (struct msgbuf *)&sndmsg, sizeof(str2)+1, 0) == -1)  
    {
    	printf ("msgsnd error");
    }
  
    if((status = msgrcv(msgid, (struct msgbuf*)&rcvmsg, 128, 100, IPC_NOWAIT)) == -1)  
    {
    	printf ("msgrcv error");
    }
  
    printf("Recevied message:\n%s\n", rcvmsg.msgtext);  
    if((status = msgrcv(msgid, (struct msgbuf*)&rcvmsg, 128, 200, IPC_NOWAIT)) == -1)  
    {
    	printf ("msgrcv error");
    }
  
    printf("Recevied message:\n%s\n", rcvmsg.msgtext);  
    msgctl(msgid, IPC_RMID, 0);
    
    exit(0);  
}  
