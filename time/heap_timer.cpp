#include "heap_timer.h"

void HeapTimer::swap_node_(size_t i, size_t j)
{
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());

    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

bool HeapTimer::sift_up_(size_t index)
{
    assert(index >= 0 && index < heap_.size());

    size_t i = index;
    size_t j = (index - 1) / 2;

    while(j >= 0) 
    {
        if(heap_[j] < heap_[i]) 
            break;
        swap_node_(i, j);
        i = j;
        j = (i - 1) / 2;
    }

    return i < index;
}

bool HeapTimer::sift_down_(size_t index, size_t n)
{
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());

    size_t i = index;
    size_t j = i * 2 + 1;

    while(j < n) 
    {
        if(j + 1 < n && heap_[j + 1] < heap_[j]) 
            j++;
        
        if(heap_[i] < heap_[j]) 
            break;
        swap_node_(i, j);
        
        i = j;
        j = i * 2 + 1;
    }

    return i > index;
}

void HeapTimer::delete_(size_t index) 
{
    assert(!heap_.empty() && index >= 0 && index < heap_.size());

    size_t i = index;
    size_t n = heap_.size() - 1;

    assert(i <= n);
    
    if(i < n) 
    {
        swap_node_(i, n);
        if(!sift_down_(i, n)) 
        {
            sift_up_(i);
        }
    }

    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::push(int id, int timeout, const timeout_callback& cb) 
{
    if(ref_.find(id) == ref_.end()) 
    {
        size_t i = heap_.size();
        ref_[id] = i;           // ref_.insert({id, i});
        heap_.push_back({id , Clock::now() + ms(timeout), cb});

        sift_up_(i); 
    } 
    else 
    {
        size_t i = ref_[id];
        heap_[i].expires = Clock::now() + ms(timeout);
        heap_[i].cb = cb;
        if(!sift_down_(i, heap_.size())) 
        {
            sift_up_(i);
        }
    }
}

void HeapTimer::pop() 
{
    assert(!heap_.empty());
    delete_(0);
}

void HeapTimer::clear() 
{
    ref_.clear();
    heap_.clear();
}

void HeapTimer::adjust(int id, int timeout) 
{
    assert(!heap_.empty() && ref_.find(id) != ref_.end());

    heap_[ref_[id]].expires = Clock::now() + ms(timeout);;
    sift_down_(ref_[id], heap_.size());
}

void HeapTimer::tick() 
{
    if(heap_.empty())
        return;

    while(!heap_.empty()) 
    {
        timer_node node = heap_.front();
        if(std::chrono::duration_cast<ms>(node.expires - Clock::now()).count() > 0) 
            break;

        node.cb();
        pop();
    }
}

int HeapTimer::get_next_tick() 
{
    tick();
    size_t res = -1;
    if(!heap_.empty()) {
        res = std::chrono::duration_cast<ms>(heap_.front().expires - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;
}

void HeapTimer::do_work(int id) 
{
    if(heap_.empty() || ref_.find(id) == ref_.end()) 
    {
        return;
    }
    size_t i = ref_[id];
    timer_node node = heap_[i];

    node.cb();
    delete_(i);
}