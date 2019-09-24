#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const char *fifo1 = "fifo_r";
const char *fifo2 = "fifo_w";

int main()
{
    int		readfd, wfd;
    int flag = 0;
    if (mkfifo (fifo1, 0644)<0){
        if (errno == EEXIST){
            //printf ("EEXIST\n");
            readfd = open (fifo1, O_RDONLY);
            wfd = open(fifo1, O_WRONLY); // 没有用，但是要加。
            flag = 1;
        }
    }
    if (flag == 0){
        readfd = open (fifo1, O_RDONLY);
        wfd = open(fifo1, O_WRONLY);
    }
        
    char buf[1024];
    int n = 0;
    while ((n = read (readfd, buf, 1024)) > 0){
        buf[n] = '\0';
        printf ("buf:%s\n", buf);
    }
    close (readfd);
    //unlink (fifo1);

    return 0;
}