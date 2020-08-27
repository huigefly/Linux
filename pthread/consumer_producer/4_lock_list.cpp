#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <list>
#include <string>
using namespace std;

#define MAX_BUF_SIZE 100

typedef long unsigned int LUINT_T;

class CAutoLock
{
public:
    CAutoLock(pthread_mutex_t *plock):m_lock(plock){
        pthread_mutex_lock(m_lock);
    }

    ~CAutoLock(){
        pthread_mutex_unlock(m_lock);
    }
private:
    pthread_mutex_t* m_lock;
};

template<typename T>
class CList
{
public:
    LUINT_T size(){
        CAutoLock lock(&m_lock);
        return m_list.size();
    }

    void push_back(const T& data){
        CAutoLock lock(&m_lock);
        m_list.push_back(data);
    }

    T front(){
        CAutoLock lock(&m_lock);
        if (m_list.size() > 0){
            T t = m_list.front();
            m_list.pop_front();
            return t;
        }
        return T();
    }

    CList():m_lock(PTHREAD_MUTEX_INITIALIZER){
    }
private:
    list<T> m_list;
    pthread_mutex_t m_lock;
};

CList<string> g_list;

void *proc_producer(void *lparam)
{
    int i = 0;
    unsigned int tid =pthread_self();
    while (1) {
        // wait list not full
        if (g_list.size() == MAX_BUF_SIZE) {
            while (1) {
                if (g_list.size() < MAX_BUF_SIZE) {
                    break;
                }
                sleep(1);
            }          
        }
        char szBuf[64] = {0};
        sprintf(szBuf, "producer_[%u]_[%d]", tid, i++);

        // push data
        if (g_list.size() < MAX_BUF_SIZE) {
            g_list.push_back(szBuf); 
        }

        printf("proc_producer tid:%u, buf:%s, list size:%lu\n", tid, szBuf, g_list.size());
        sleep(1);
    }
}

void *proc_consumer(void *lparam)
{
    while (1) {
        // empty sleep
        if (g_list.size() == 0) {
            while (1) {
                if (g_list.size() > 0) {
                    break;
                }
                sleep(1);
            }
        }
        
        // get data
        string sBuf = g_list.front();
        if (sBuf.size() > 0)
            printf(" proc_consumer get:%s\n", sBuf.c_str());
        printf(" proc_consumer list size:%lu\n", g_list.size());
        usleep(1000 * 1);
    }
}

int main()
{
    pthread_t tid_consumer, tid_producer;
    int i = 0;
    for (i=0; i<10; i++){
        if (-1 == pthread_create(&tid_producer, NULL, proc_producer, NULL)){
            return -1;
        }
    }

    for (i=0; i<10; i++){
        if (-1 == pthread_create(&tid_consumer, NULL, proc_consumer, NULL)){
            return -1;
        }
    }

    sleep (520 * 1314);

    return 0;
}