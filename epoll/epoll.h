#ifndef EPOLL_H
#define EPOLL_H
#include <sys/epoll.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <assert.h>  
#include <vector>
#include <errno.h>

static const int MAX_EVENT_NUMBER = 10000;

class Epoll 
{
public:

    static Epoll *GetInstance(int max_event = 1024);
    
    bool add_fd(int fd, uint32_t events);

    bool mod_fd(int fd, uint32_t events);

    bool del_fd(int fd);

    int wait(int timeoutMs = -1);

    int get_event_fd(size_t i) const;

    uint32_t get_events(size_t i) const;
    
    void destory_epoll();

private:
    explicit Epoll(int max_event);
    ~Epoll();

private:
    int epoll_fd_;
    static Epoll* epoll_;                       
    epoll_event events_[MAX_EVENT_NUMBER];
};
#endif  //EPOLL_H