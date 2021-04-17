#include "epoll.h"

Epoll *Epoll::GetInstance(int max_event)
{
    static Epoll epoll(max_event);
    return &epoll;
}

Epoll::Epoll(int max_event)
{
    epoll_fd_ = epoll_create(512);
    assert(epoll_fd_ >= 0);
}

Epoll::~Epoll() 
{
    close(epoll_fd_);
}

bool Epoll::add_fd(int fd, uint32_t events) {
    if(fd < 0) 
        return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoll::mod_fd(int fd, uint32_t events) {
    if(fd < 0) 
        return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoll::del_fd(int fd) 
{
    if(fd < 0) 
        return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &ev);
}

int Epoll::wait(int timeout_ms) 
{
    return epoll_wait(epoll_fd_, &events_[0], static_cast<int>(MAX_EVENT_NUMBER), timeout_ms);
}

int Epoll::get_event_fd(size_t i) const 
{
    assert(i < MAX_EVENT_NUMBER && i >= 0);
    return events_[i].data.fd;
}

uint32_t Epoll::get_events(size_t i) const 
{
    assert(i < MAX_EVENT_NUMBER && i >= 0);
    return events_[i].events;
}