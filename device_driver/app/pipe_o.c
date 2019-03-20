#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../chat_mult/chat.h"

int main(int argc, char *argv[])
{
  int pipefd[2];
  pid_t cpid;
  
	if (pipe(pipefd) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}
	
	int fd = open (argv[1], O_WRONLY);
	if (-1 != fd) {
	    printf ("start hook\n");
		ioctl (fd, CHAT_IOC_HOOK);
		printf ("start unhook\n");
	}

	close (fd);

	return 0;
}