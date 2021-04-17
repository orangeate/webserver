#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/types.h>
#include <sys/uio.h>
#include <arpa/inet.h> 
#include <stdlib.h>
#include <errno.h> 

#include <string>
#include <assert.h>

#include "../log/log.h"
#include "../buffer/buffer.h"
#include "../epoll/epoll.h"
#include "http_request.h"
#include "http_response.h"

class HttpConn 
{
public:
    HttpConn();
    ~HttpConn() = default;

    void init(int sockFd, uint32_t _conn_event, const sockaddr_in& addr);

    void close_();

    bool process();
    bool process_read();
    bool process_write();

    int get_fd() const;
    const char* get_IP() const;
    int get_port() const;
   
    static bool is_et_;
    static std::string root_dir_;
    static std::atomic<int> user_count;     // 连接数

    int state_;

private:
    int write_bytes() { return iov_[0].iov_len + iov_[1].iov_len;}

private:
    int fd_;
    struct  sockaddr_in addr_;

    uint32_t conn_event_;                   // 连接的文件描述符事件
    Epoll* epoll_;
    
    bool is_close_;

    int iov_cnt_;
    struct iovec iov_[2];

    Buffer read_buffer_;
    Buffer write_buffer_;

    HttpRequest request_;
    HttpResponse response_;
};

#endif //HTTP_CONN_H