#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const char *fifo1 = "fifo_r";
const char *fifo2 = "fifo_r";

int main()
{
    int		fd;
    if (mkfifo (fifo2, 0644)<0){
        if (errno == EEXIST){
            //printf ("EEXIST\n");
            fd = open (fifo2, O_WRONLY);
        }
    }

    while (1){
        char buf[1024];
        sprintf (buf, "hellwoorodl_%d", rand());
        write (fd, buf, 1024);
        sleep (1);
    }

    close (fd);
    unlink (fifo1);

    return 0;
}