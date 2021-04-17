#include "lock.h"

Semaphore::Semaphore()
{
    if (sem_init(&semaphore_, 0, 0) != 0)
    {
        throw std::exception();
    }
}

Semaphore::Semaphore(int num)
{
    if (sem_init(&semaphore_, 0, num) != 0)
    {
        throw std::exception();
    }
}

Semaphore::~Semaphore()
{
    sem_destroy(&semaphore_);
}

bool Semaphore::wait()
{
    return sem_wait(&semaphore_) == 0;
}

bool Semaphore::post()
{
    return sem_post(&semaphore_) == 0;
}

Mutex::Mutex()
{
    if (pthread_mutex_init(&mutex_, NULL) != 0)
    {
        throw std::exception();
    }
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&mutex_);
}

bool Mutex::lock()
{
    return pthread_mutex_lock(&mutex_) == 0;
}

bool Mutex::unlock()
{
    return pthread_mutex_unlock(&mutex_) == 0;
}

pthread_mutex_t* Mutex::get()
{
    return &mutex_;
}

Condition::Condition()
{
    if (pthread_cond_init(&cond_, NULL) != 0)
    {
        //pthread_mutex_destroy(&m_mutex);
        throw std::exception();
    }
}

Condition::~Condition()
{
    pthread_cond_destroy(&cond_);
}

bool Condition::wait(pthread_mutex_t *m_mutex)
{
    int ret = 0;
    //pthread_mutex_lock(&m_mutex);
    ret = pthread_cond_wait(&cond_, m_mutex);
    //pthread_mutex_unlock(&m_mutex);
    return ret == 0;
}

bool Condition::timewait(pthread_mutex_t *m_mutex, struct timespec t)
{
    int ret = 0;
    //pthread_mutex_lock(&m_mutex);
    ret = pthread_cond_timedwait(&cond_, m_mutex, &t);
    //pthread_mutex_unlock(&m_mutex);
    return ret == 0;
}

bool Condition::signal()
{
    return pthread_cond_signal(&cond_) == 0;
}

bool Condition::broadcast()
{
    return pthread_cond_broadcast(&cond_) == 0;
}