#include <unistd.h>  
#include <stdlib.h>  
#include <fcntl.h>  
#include <limits.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <stdio.h>  
#include <string.h>  

int main()  
{  
    const char *fifo_name = "./fifotest";  
    int pipe_fd = -1;  
    int data_fd = -1;  
    int res = 0;  
    const int open_mode = O_WRONLY;  
    int bytes_sent = 0;  
    char buffer[PIPE_BUF + 1];  

    if(access(fifo_name, F_OK) == -1)  
    {  
        //�ܵ��ļ�������  
        //���������ܵ�  
        res = mkfifo(fifo_name, 0777);  
        if(res != 0)  
        {  
            fprintf(stderr, "Could not create fifo %s\n", fifo_name);  
            exit(EXIT_FAILURE);  
        }  
    }  

    printf("Process %d opening FIFO O_WRONLY\n", getpid());  
    //��ֻд������ʽ��FIFO�ļ�����ֻ����ʽ�������ļ�  
    pipe_fd = open(fifo_name, open_mode);  
    data_fd = open("Data.txt", O_RDONLY);  
    printf("Process %d result %d\n", getpid(), pipe_fd);  

    if(pipe_fd != -1)  
    {  
        int bytes_read = 0;  
        //�������ļ���ȡ����  
        bytes_read = read(data_fd, buffer, PIPE_BUF);  
        buffer[bytes_read] = '\0';  
        while(bytes_read > 0)  
        {  
            //��FIFO�ļ�д����  
            res = write(pipe_fd, buffer, bytes_read);  
            if(res == -1)  
            {  
                fprintf(stderr, "Write error on pipe\n");  
                exit(EXIT_FAILURE);  
            }  
            //�ۼ�д���ֽ�������������ȡ����  
            bytes_sent += res;  
            bytes_read = read(data_fd, buffer, PIPE_BUF);  
            buffer[bytes_read] = '\0';  
        }  
        close(pipe_fd);  
        close(data_fd);  
    }  
    else  
        exit(EXIT_FAILURE);  

    printf("Process %d finished\n", getpid());  
    exit(EXIT_SUCCESS);  
}  