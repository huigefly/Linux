#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
   int pipefd[2];
   pid_t cpid;
   
   

   if (pipe(pipefd) == -1) {
       perror("pipe");
       exit(EXIT_FAILURE);
   }

   cpid = fork();
   if (cpid == -1) {
       perror("fork");
       exit(EXIT_FAILURE);
   }

   if (cpid == 0) {    									/* Child reads from pipe */
       close(pipefd[1]);          			/* Close unused write end */
			 char szBuf[256] = {0};
       while (read(pipefd[0], &szBuf, 256) > 0) {
       	 sprintf (szBuf, "%s\n", szBuf);
       	 write(STDOUT_FILENO, &szBuf, 256);
       }
       printf ("child exit\n");
       write(STDOUT_FILENO, "\n", 1);
       close(pipefd[0]);
       _exit(EXIT_SUCCESS);

   } else {            						/* Parent writes argv[1] to pipe */
       close(pipefd[0]);          /* Close unused read end */
       char szBuf[256] = {0};
       FILE *fp = fopen ("data_test_pipe", "r");
       while (fgets (szBuf, 256, fp)) {
       	 printf ("parent ready write::%s\n", szBuf);
       	 write(pipefd[1], szBuf, 256);
       	 memset (szBuf, 0, 256);
       }
       printf ("parent exit\n");
       fclose (fp);
       close(pipefd[1]);          /* Reader will see EOF */
       wait(NULL);                /* Wait for child */
       exit(EXIT_SUCCESS);
   }
   
   return 0;
}