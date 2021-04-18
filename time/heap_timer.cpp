#include "heap_timer.h"

void HeapTimer::swap_node_(size_t i, size_t j)
{
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());

    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

void HeapTimer::sift_up_(size_t i)
{
    assert(i >= 0 && i < heap_.size());

    size_t p = (i - 1) / 2;
    while(p >= 0) 
    {
        if(heap_[p] < heap_[i]) 
            break;
        swap_node_(i, p);
        i = p;
        p = (i - 1) / 2;
    }
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
        if(!sift_down_(i, n)) {
            sift_up_(i);
        }
    }

    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::push(int id, int timeout, const timeout_callback& cb) 
{
    size_t i;

    if(ref_.find(id) == ref_.end()) 
    {
        i = heap_.size();
        ref_[id] = i;           // ref_.insert({id, i});
        heap_.push_back({id , Clock::now() + ms(timeout), cb});

        sift_up_(i); 
    } 
    else 
    {
        i = ref_[id];
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
    assert(!heap_.empty() && ref_.count(id) > 0);

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

int HeapTimer::get_bext_tick() 
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