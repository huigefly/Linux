#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

#define	NBUFF	 10
#define	sem_MUTEX	"mutex"	 	/* these are args to px_ipc_name() */
#define	sem_NEMPTY	"nempty"
#define	sem_NSTORED	"nstored"

int		nitems;					/* read-only by producer and consumer */
struct {	/* data shared by producer and consumer */
  int	buff[NBUFF];
  int 	index;
  int 	value;
  sem_t	*mutex, *nempty, *nstored;
} shared;

void	*produce(void *), *consume(void *);

int
main(int argc, char **argv)
{
	

	if (argc != 2){
		printf ("usage: prodcons1 <#items>");
        exit (0);
    }
	nitems = atoi(argv[1]);

	/* 4create three semaphores */
loop1:
	shared.mutex = sem_open("mutex1", O_CREAT | O_EXCL,
							0644, 1);
	if (SEM_FAILED == shared.mutex){
		if (EEXIST == errno){
			sem_unlink("mutex1");
            goto loop1;
		}
	}
loop2:
	shared.nempty = sem_open("nempty1", O_CREAT | O_EXCL,
							 0644, NBUFF);
    if (SEM_FAILED == shared.nempty){
		if (EEXIST == errno){
			sem_unlink("nempty1");
            goto loop2;
		}
	}
loop3:
	shared.nstored = sem_open("nstored1", O_CREAT | O_EXCL,
							  0644, 0);
    if (SEM_FAILED == shared.nstored){
		if (EEXIST == errno){
			sem_unlink("nstored1");
            goto loop3;
		}
	}
    pthread_t	tid_produce[10], tid_consume;
	/* 4create one producer thread and one consumer thread */
	for (int i=0; i<10; i++){
		pthread_create(&tid_produce[i], NULL, produce, NULL);
	}
	pthread_create(&tid_consume, NULL, consume, NULL);

	/* 4wait for the two threads */
	for (int i=0; i<10; i++){
		pthread_join(tid_produce[i], NULL);
	}
	pthread_join(tid_consume, NULL);

	/* 4remove the semaphores */
	sem_unlink("mutex1");
	sem_unlink("nempty1");
	sem_unlink("nstored1");
	exit(0);
}
/* end main */

/* include prodcons */
void *
produce(void *arg)
{
	while (1) {
		sem_wait(shared.nempty);	/* wait for at least 1 empty slot */
		sem_wait(shared.mutex);
		if (shared.index >= nitems){
			sem_post(shared.mutex);
			sem_post(shared.nempty);
			return NULL;
		}
		shared.buff[shared.index % NBUFF] = shared.value;	/* store i into circular buffer */
		shared.index++;
		shared.value++;
		sem_post(shared.mutex);
		sem_post(shared.nstored);	/* 1 more stored item */
        //printf ("produce i :%d\n", i);
	}
	return(NULL);
}

void *
consume(void *arg)
{
	int		i;
	for (i = 0; i < nitems; i++) {
		sem_wait(shared.nstored);		/* wait for at least 1 stored item */
		sem_wait(shared.mutex);
		if (shared.buff[i % NBUFF] != i)
			printf("buff[%d] = %d\n", i, shared.buff[i % NBUFF]);
		sem_post(shared.mutex);
		sem_post(shared.nempty);		/* 1 more empty slot */
        //printf("consumer, buff[%d] = %d\n", i, shared.buff[i % NBUFF]);
	}
	return(NULL);
}
/* end prodcons */
