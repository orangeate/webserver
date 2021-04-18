#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>

typedef std::function<void()> timeout_callback;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds ms;
typedef Clock::time_point time_stamp;

struct timer_node 
{
    int id;
    time_stamp expires;
    timeout_callback cb;

    bool operator<(const timer_node& t) 
    {
        return expires < t.expires;
    }
};

class HeapTimer 
{
public:
    HeapTimer() 
    { 
        heap_.reserve(64); 
    }
    ~HeapTimer() 
    { 
        clear(); 
    }

    void push(int id, int timeOut, const timeout_callback& cb);
    void pop();
    void clear();
    void adjust(int id, int newExpires);

    void do_work(int id);

    void tick();    
    int get_bext_tick();


private:
    void swap_node_(size_t i, size_t j);
    void sift_up_(size_t i);
    bool sift_down_(size_t index, size_t n);
    void delete_(size_t i);
    
    std::vector<timer_node> heap_;
    std::unordered_map<int, size_t> ref_;           // timer_node node = heap_[ ref_[id] ]
};

#endif //HEAP_TIMER_H
