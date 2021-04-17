#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include <vector>
#include "../lock/lock.h"

template <typename T>
class ThreadPool
{
public:

    ThreadPool(int thread_number = 8, int max_request = 10000);
    ~ThreadPool();
    bool append(T *request, int state);

private:
    /* 工作线程运行的函数 */
    static void *worker(void *arg);
    void run();

private:
    pthread_t *p_threads_;           //描述线程池的数组
    int thread_number_;              //线程池中的线程数

    std::list<T *> request_queue_;   //请求队列
    int max_request_;                //请求队列中允许的最大请求数
    
    Mutex mutex_;                    //保护请求队列的互斥锁
    Semaphore sem_;                  //是否有任务需要处理
    
    bool stop_;                      //是否结束线程
};

/*
    线程池大小 创建线程池
    请求队列大小
*/
template <typename T>
ThreadPool<T>::ThreadPool(int thread_number, int max_requests):
    thread_number_(thread_number), max_request_(max_requests) 
{
    stop_ = false;
    p_threads_ = nullptr;

    if (thread_number_ <= 0 || max_request_ <= 0)
        throw std::exception();
    p_threads_ = new pthread_t[thread_number_];

    if (!p_threads_)
        throw std::exception();

    // 创建 thread_number 个线程
    for (int i = 0; i < thread_number; ++i)
    {
        // 创建线程
        if (pthread_create(p_threads_ + i, NULL, worker, this) != 0)
        {
            delete[] p_threads_;
            throw std::exception();
        }
        // 分离线程
        if (pthread_detach(p_threads_[i]))
        {
            delete[] p_threads_;
            throw std::exception();
        }
    }
}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
    // 销毁线程池
    delete[] p_threads_;
    stop_ = true;
}

template <typename T>
void *ThreadPool<T>::worker(void *arg)
{
    // 执行任务
    ThreadPool *pool = (ThreadPool *)arg;
    pool->run();
    return pool;
}

template <typename T>
void ThreadPool<T>::run()
{
    while (!stop_)
    {
        sem_.wait();
        mutex_.lock();
        if (request_queue_.empty())
        {
            mutex_.unlock();
            continue;
        }

        T *request = request_queue_.front();
        request_queue_.pop_front();
        mutex_.unlock();

        if (!request)
            continue;
        
        // 处理读
        if (request->state_ == 0)
        {
            request->process_read();
            
        }
        // 处理写
        else if(request->state_ == 1)
        {
            request->process_write();        
        }
        else
        {
            continue;
        }
    }
}

template <typename T>
bool ThreadPool<T>::append(T *request, int state)
{
    mutex_.lock();
    // 请求队列满
    if (request_queue_.size() > max_request_)
    {
        mutex_.unlock();
        return false;
    }
    request->state_ = state;
    request_queue_.push_back(request);
    mutex_.unlock();

    sem_.post();
    return true;
}

#endif //THREAD_POOL_H