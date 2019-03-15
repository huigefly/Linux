#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	int fd = open (argv[1], O_RDWR,S_IRUSR|S_IWUSR);
	if (-1 != fd) {
		char szBuf[256] = {0};
		printf ("1 errno:%d, EINTR:%d\n", errno, EINTR);
		
		for (;;) {
			int nRtn = 0;
			while (((nRtn = read (fd, szBuf, 256)) < 0) && (errno == EINTR || errno == EAGAIN)) {
 		        	printf ("in while nRtn:%d, errno:%d\n", nRtn, errno);
 		        	sleep (1);
			}
			sleep (1);
			printf ("out errno:%d, buf:%s, nRtn:%d\n", errno, szBuf, nRtn);
		}
	}

	close (fd);

	return 0;
}

