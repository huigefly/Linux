#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int my_lock(int fd)
{
	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;				/* write lock entire file */
	return fcntl(fd, F_SETLKW, &lock);  // wait
}

int my_unlock(int fd)
{
	struct flock lock;
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;				/* unlock entire file */
	return fcntl(fd, F_SETLK, &lock);
}

int main()
{
    int fd = open ("record", O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    // if (my_lock (fd) < 0){
    //     if (errno == EACCES || errno == EAGAIN)
    //         printf("unable to lock, is already running?\n");
    //     else
    //         printf("unable to lock \n");
    //     printf ("lock err, pid:%d\n", getpid ());
    // }
    // sleep (100);
/*
    写同步
*/
#if 1
    for (int i=0; i<20; i++) {
        printf ("ready lock, pid:%d\n", getpid ());
        if (my_lock (fd) < 0){
            if (errno == EACCES || errno == EAGAIN)
                printf("unable to lock, is already running?\n");
            else
                printf("unable to lock \n");
            printf ("lock err, pid:%d\n", getpid ());
        }
        lseek(fd, 0L, SEEK_SET);
        char buf[1024];
        int n = read(fd, buf, 1024);
        buf[n] = '\0';
        int no = atoi (buf);
        no++;
        printf ("pid:%d,buf:%d\n", getpid(), no);
        snprintf(buf, sizeof(buf), "%d\n", no);
        sleep (1);
        lseek(fd, 0L, SEEK_SET);
        write (fd, buf, strlen (buf));
        my_unlock (fd);
    }
#endif // 0
    close (fd);

    return 0;
}