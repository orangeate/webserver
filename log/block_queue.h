#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <deque>
#include <sys/time.h>
#include <assert.h>

#include "../lock/lock.h"

template<class T>
class BlockDeque {
public:
    explicit BlockDeque(size_t capacity = 1000);

    ~BlockDeque();

    void clear();
    bool empty();
    bool full();
    void close_();

    size_t size();
    size_t capacity();

    T front();
    T back();

    void push_back(const T &item);
    void push_front(const T &item);

    bool pop(T &item);
    bool pop(T &item, int timeout);

    void flush();

private:

    std::deque<T> deque_;
    size_t capacity_;

    bool is_close_;
    Mutex mutex_;

    Condition cond_producer_;
    Condition cond_consumer_;
};

template<class T>
BlockDeque<T>::BlockDeque(size_t capacity) : capacity_(capacity)
{
    assert(capacity > 0);
    is_close_ = false;
}

template<class T>
BlockDeque<T>::~BlockDeque()
{
    close_();
};

template<class T>
void BlockDeque<T>::close_()
{
    mutex_.lock();
    deque_.clear();
    is_close_ = true;
    mutex_.unlock();

    cond_producer_.broadcast();
    cond_consumer_.broadcast();
};

template<class T>
void BlockDeque<T>::flush()
{
    cond_consumer_.signal();
};

template<class T>
void BlockDeque<T>::clear()
{
    mutex_.lock();
    deque_.clear();
    mutex_.unlock();
}

template<class T>
T BlockDeque<T>::front()
{
    mutex_.lock();
    auto val = deque_.front();
    mutex_.unlock();

    return val;
}

template<class T>
T BlockDeque<T>::back()
{
    mutex_.lock();
    auto val =  deque_.back();
    mutex_.unlock();

    return val;
}

template<class T>
size_t BlockDeque<T>::size()
{
    mutex_.lock();
    auto val = deque_.size();
    mutex_.unlock();

    return val;
}

template<class T>
size_t BlockDeque<T>::capacity()
{
    return capacity_;
}

template<class T>
void BlockDeque<T>::push_back(const T &item)
{
    mutex_.lock();
    while(deque_.size() >= capacity_)
    {
        cond_producer_.wait(mutex_.get());
    }
    mutex_.unlock();

    deque_.push_back(item);
    cond_consumer_.signal();
}

template<class T>
void BlockDeque<T>::push_front(const T &item)
{
    mutex_.lock();
    while(deque_.size() >= capacity_)
    {
        cond_producer_.wait(mutex_.get());
    }
    mutex_.unlock();

    deque_.push_front(item);
    cond_consumer_.signal();
}

template<class T>
bool BlockDeque<T>::empty()
{
    mutex_.lock();
    auto val = deque_.empty();
    mutex_.unlock();

    return val;
}

template<class T>
bool BlockDeque<T>::full()
{
    mutex_.lock();
    auto val =  deque_.size() >= capacity_;
    mutex_.unlock();

    return val;
}

template<class T>
bool BlockDeque<T>::pop(T &item)
{
    mutex_.lock();

    while(deque_.empty())
    {
        cond_consumer_.wait(mutex_.get());
        if(is_close_)
        {
            return false;
        }
    }
    item = deque_.front();
    deque_.pop_front();

    mutex_.unlock();


    cond_producer_.signal();

    return true;
}

// template<class T>
// bool BlockDeque<T>::pop(T &item, int timeout)
// {
//     mutex_.lock();

//     while(deque_.empty()){
//         if(cond_consumer_.timewait(mutex_.get(), timeout))
//         {
//             return false;
//         }
//         if(is_close_)
//         {
//             return false;
//         }
//     }
//     item = deque_.front();
//     deque_.pop_front();
//     mutex_.unlock();

//     cond_producer_.signal();

//     return true;
// }

#endif // BLOCKQUEUE_H