#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define READ_INDEX 		0
#define WRITE_INDEX		1

int main(int argc, char *argv[])
{
	int pipefd[2];
	pid_t cpid;
	char buf;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <string>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (pipe(pipefd) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	cpid = fork();
	if (cpid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (cpid == 0) {    			/* Child reads from pipe */
		close(pipefd[WRITE_INDEX]);          	/* Close unused write end */

		while (read(pipefd[READ_INDEX], &buf, 1) > 0){
			//printf("child recv parent send:%s\n", buf)
			write(STDOUT_FILENO, &buf, 1);
		}

		write(STDOUT_FILENO, "\n", 1);
		close(pipefd[READ_INDEX]);
		_exit(EXIT_SUCCESS);

	} else {            			/* Parent writes argv[1] to pipe */
		close(pipefd[READ_INDEX]);          	/* Close unused read end */
		write(pipefd[WRITE_INDEX], argv[1], strlen(argv[1]));
		close(pipefd[WRITE_INDEX]);          	/* Reader will see EOF */
		printf("parent wati child returning!\n");
		wait(NULL);                	/* Wait for child */
		printf("parent wati child return ok!\n");
		exit(EXIT_SUCCESS);
	}
	return 0;
}