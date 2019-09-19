#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <pthread.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <semaphore.h>
using namespace std;

#define info(format, ...) printf ("%s:%d <INFO>"#format"\n", __FUNCTION__, __LINE__, #__VA_ARGS__)

vector<string> vecGlobal;

pthread_mutex_t g_mutex;
sem_t g_sem;
sem_t g_sem2;
sem_t g_sem3;

void *test_one_proc(void *lparam)
{
	int nId = *(int *)lparam;
	printf ("nID:%d\n", nId);
	if (1 == nId)
	{
		int i = 0;
		while (1)
		{
			char szHello[256] = {0};
			sprintf (szHello, "%s %d", "hello", i++);
			
			for (int k=0; k<5; k++)
			{
				pthread_mutex_lock (&g_mutex);
				sleep (3);
				vecGlobal.push_back (szHello);
				pthread_mutex_unlock (&g_mutex);
				printf ("---- thread 1:id:%d, push_back:%s\n", k, szHello);
				sleep (1);			
			}
//			sleep (3);
		}
	}
	else 
	{
		while (1)
		{
			if (vecGlobal.empty () != true)
			{
//				printf ("enter thread 2\n");
				vector<string> vecTemp;
				pthread_mutex_lock (&g_mutex);
				vecTemp.assign (vecGlobal.begin (), vecGlobal.end ());
				pthread_mutex_unlock (&g_mutex);
				
				int i=0, nSize;
				nSize = vecTemp.size ();
				for (i=0; i<nSize; i++)
				{
					printf ("thread 2:%d, %s\n", i, vecTemp[i].c_str ());
				}
//				vecGlobal.push_back ("i am testing");
//				printf ("id:%d, %s\n", nId, vecGlobal.back ().c_str ());
			}
			sleep (1);
		}
	}

	pthread_exit ((void *)0);
}

void *test1_signal_proc(void *lparam)
{
	printf ("enter pthread 1\n");
	while (1)
	{
//		if (0 == sem_wait (&g_sem))
		if (0 == sem_trywait (&g_sem3))
		{
			printf ("1....i am waiting for signal!\n");		
		}
		printf ("1 waiting!\n");
		sleep (3);
		
		/* ����һ��ѭ�����źŷ��ͻ��� */
		#if 0
		sleep (3);
		sem_post (&g_sem3);
		#endif //0
	}

	pthread_exit (0);
}

void *test2_signal_proc(void *lparam)
{
	printf ("enter pthread 2\n");
	while (1)
	{
//		if (0 == sem_wait (&g_sem2))
		if (0 == sem_trywait (&g_sem3))
		{
			printf ("2....i am waiting for signal!\n");		
		}
		sem_post (&g_sem);
		printf ("2 waiting!\n");
		sleep (3);
	}

	pthread_exit (0);
}

void *test3_signal_proc(void *lparam)
{
	printf ("enter pthread 3\n");
	while (1)
	{
		/* ���ó�ʱ */
		#if 0
		struct timespec ts;
		ts.tv_sec = time (NULL) + 5;
		if (-1 == sem_timedwait (&g_sem3, &ts))
		{
			printf ("timeout\n");
			continue;
		}
		#endif //0
		
//		if (0 == sem_wait (&g_sem3))
		if (0 == sem_trywait (&g_sem3))
		{
			printf ("3....i am waiting for signal!\n");		
		}
		sem_post (&g_sem2);
		printf ("3 waiting!\n");
		sleep (3);
	}

	pthread_exit (0);
}

int executeCMD(const char *szCmd, char *szResult, size_t nResultLength)
{
    /* check */
    if ((NULL == szCmd) || (NULL == szResult) || (nResultLength <= 0))
    {
    	return -1;
    }
    memset(szResult, 0, nResultLength);
    
    /* open */
    FILE *fp = NULL;;
    if(NULL == (fp = popen(szCmd, "r")))
    {
        printf ("popen %s error\n", szCmd);
        return -2;
    }
    
    /* read */
    fread(szResult, nResultLength-1, 1, fp);
    
    /* close */
    pclose(fp);
    
    return 0;
}

int ExecCmdBySystem(const char *szCmd, const char *szTempFile)
{
	if ((NULL == szCmd) || (NULL == szTempFile))
	{
		return -1;
	}
	
	char szNewCmd[1024] = {0};
	
	sprintf (szNewCmd, "%s > %s", szCmd, szTempFile);
	
	/* exec */
	return system (szNewCmd);
}

int ExecCmd(const string& sCmd, string *psResult)
{
	if ((NULL == psResult) || (sCmd.size () <= 0))
	{
		return -1;
	}
	
	if (0 != ExecCmdBySystem (sCmd.c_str (), "__temp_result__"))
	{
		return -2;
	}
	
	FILE *fp = NULL;
	
	/* open */
	if (NULL == (fp = fopen ("__temp_result__", "r")))
	{
		return -3;
	}
	
	char szLine[1024] = {0};
	while (NULL != fgets (szLine, 1024, fp))
	{
		string sTemp = szLine;
		*psResult += sTemp;
	}
	
	/* close */
	fclose (fp);
//	remove ("__temp_result__");

	return 0;
}

int main ()
{
#if 0
	string sHello = "nihao xiansheng";
	string sTest = "jfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioajfisajfioa";
	sHello.replace (sHello.find ("xiansheng"), 5, sTest);
	
	printf ("%s\n", sHello.c_str ());

	return 0;

	char szResult[1024] = {0};
	string sResult;
	
//	executeCMD ("ls", szResult, sizeof (szResult));
//	ExecCmdBySystem ("ls", "temp_r");
	ExecCmd ("ifconfig", &sResult);
	
	printf ("result:%s\n", sResult.c_str ());	
#endif //0

#if 1
	pthread_t tid1 = (pthread_t)-1;
	pthread_t tid2 = (pthread_t)-1;
	pthread_t tid3 = (pthread_t)-1;
	
	if (0 != sem_init (&g_sem, 0, 0))
	{
		printf ("sem_init error\n");
		return -1;
	}
	
	if (0 != sem_init (&g_sem2, 0, 0))
	{
		printf ("sem_init error\n");
		return -1;
	}
	
	if (0 != sem_init (&g_sem3, 0, 0))
	{
		printf ("sem_init error\n");
		return -1;
	}
	
	if (0 != pthread_create (&tid1, NULL, test1_signal_proc, NULL))
	{
		return -2;
	}
	
	if (0 != pthread_create (&tid2, NULL, test2_signal_proc, NULL))
	{
		return -3;
	}
	
	if (0 != pthread_create (&tid3, NULL, test3_signal_proc, NULL))
	{
		return -4;
	}
	
//	sleep (5);
//	sem_post (&g_sem3);
	
	while (1)
		sleep (10000);
	
	/* exit pthread */
	pthread_join (tid1, NULL);
	pthread_join (tid2, NULL);
	pthread_join (tid3, NULL);
	
	/* destroy sem_t */
	sem_destroy (&g_sem);
	sem_destroy (&g_sem2);
	sem_destroy (&g_sem3);
#endif //0
	
#if 0
	list<string> listTest;
	int i;
	
	for (i=0; i<100; i++)
	{
		listTest.push_back ("hello");
	}
	
	list<string> listTemp;
	
//	listTemp.assign (listTest.begin (), listTest.end ());
	listTemp.splice (listTemp.begin (), listTest, listTest.begin (), listTest.end ());
//	listTemp.splice (listTemp.begin (), listTest, listTemp.begin (), listTemp.end ());

	list<string>::iterator it;
	for (it=listTemp.begin (); it!=listTemp.end (); it++)
	{
		printf ("%s\n", it->c_str ());
	}
	return 0;

	pthread_t tid1 = -1;
	pthread_t tid2 = -1;
	int nParam1 = 1;
	int nParam2 = 2;
	
	pthread_mutex_init (&g_mutex, NULL);
	pthread_create (&tid1, NULL, test_one_proc, (void *)&nParam1);
	pthread_create (&tid2, NULL, test_one_proc, (void *)&nParam2);
	
	pthread_join (tid1, NULL);
	pthread_join (tid2, NULL);
	pthread_mutex_destroy (&g_mutex);

	sleep (1000000);
#endif //0

	return 0;
}
