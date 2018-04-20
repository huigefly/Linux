#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <fcntl.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <limits.h>  
#include <string.h>  

int main()  
{  
    const char *fifo_name = "./fifotest";  
    int pipe_fd = -1;  
    int data_fd = -1;  
    int res = 0;  
    int open_mode = O_RDONLY;  
    char buffer[PIPE_BUF + 1] = {0};  
    int bytes_read = 0;  
    int bytes_write = 0;  

    printf("Process %d opening FIFO O_RDONLY\n", getpid());  
    
    //��ֻ��������ʽ�򿪹ܵ��ļ���ע����fifowrite.c�ļ��е�FIFOͬ��  
    pipe_fd = open(fifo_name, open_mode);  
    
    //��ֻд��ʽ�����������ݵ��ļ�  
    data_fd = open("DataFormFIFO.txt", O_WRONLY|O_CREAT, 0644);  
    printf("Process %d result %d\n",getpid(), pipe_fd);  
    if(pipe_fd != -1)  
    {  
        do  
        {  
            //��ȡFIFO�е����ݣ��������������ļ�DataFormFIFO.txt�ļ���  
            res = read(pipe_fd, buffer, PIPE_BUF);  
            bytes_write = write(data_fd, buffer, res);  
            bytes_read += res;  
        }while(res > 0);  
        close(pipe_fd);  
        close(data_fd);  
    }  
    else
    {
    	exit(EXIT_FAILURE);  
    }

    printf("Process %d finished, %d bytes read\n", getpid(), bytes_read);  
    exit(EXIT_SUCCESS);  
}