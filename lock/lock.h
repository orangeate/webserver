#ifndef LOCK_H
#define LOCK_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

/* ---- 信号量 ----*/
class Semaphore
{
public:
    Semaphore();
    Semaphore(int num);
    ~Semaphore();

    bool wait();
    bool post();
    
private:
    sem_t semaphore_;  
};

/* ---- 互斥锁 ----*/
class Mutex
{
public:
    Mutex();
    ~Mutex();

    bool lock();
    bool unlock();

    pthread_mutex_t* get();

private:
    pthread_mutex_t mutex_;
};

/* ---- 条件变量 ----*/
class Condition
{
public:
    Condition();
    ~Condition();

    bool wait(pthread_mutex_t *m_mutex);
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t);
    bool signal();
    bool broadcast();

private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t cond_;
};

#endif //LOCK_H