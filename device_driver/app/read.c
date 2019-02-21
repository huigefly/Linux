#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
	int fd = open ("/dev/chatdev", O_RDWR,S_IRUSR|S_IWUSR);
	if (-1 != fd) {
		char szBuf[256] = {0};
		read (fd, szBuf, 256);
		printf ("buf:%s\n", szBuf);
	}

	close (fd);

	return 0;
}