#include "http_conn.h"

std::string HttpConn::root_dir_;
std::atomic<int> HttpConn::user_count;
bool HttpConn::is_et_;

HttpConn::HttpConn()
{ 
    fd_ = -1;
    addr_ = { 0 };
    is_close_ = true;
    epoll_ = Epoll::GetInstance();
};

void HttpConn::init(int fd, uint32_t conn_event, const sockaddr_in& addr) 
{
    assert(fd > 0);
    user_count++;
    addr_ = addr;
    fd_ = fd;
    read_buffer_.retrieve_all();
    write_buffer_.retrieve_all();
    is_close_ = false;
    conn_event_ = conn_event;

    LOG_INFO("Client[%d](%s:%d) in, user_count:%d", fd_, get_IP(), get_port(), (int)user_count);
}

void HttpConn::close_() 
{
    response_.unmap_file();
    if(is_close_ == false)
    {
        is_close_ = true; 
        user_count--;
        close(fd_);

        LOG_INFO("Client[%d](%s:%d) quit, user_count:%d", fd_, get_IP(), get_port(), (int)user_count);
    }
}

int HttpConn::get_fd() const
{
    return fd_;
}

const char* HttpConn::get_IP() const 
{
    return inet_ntoa(addr_.sin_addr);
}

int HttpConn::get_port() const 
{
    return addr_.sin_port;
}


bool HttpConn::process()
{
    request_.init();
    if(read_buffer_.readable_bytes() < 0)
    {
        return false;
    }

    if(request_.parse(read_buffer_)) 
    {
        response_.init(root_dir_, request_.path(), request_.is_keep_alive(), 200);
    }
    else
    {
        response_.init(root_dir_, request_.path(), false, 400);
    }

    response_.make_response(write_buffer_);

    /* 响应头 */
    iov_[0].iov_base = const_cast<char*>(write_buffer_.begin_read_const());
    iov_[0].iov_len = write_buffer_.readable_bytes();
    iov_cnt_ = 1;

    /* 文件 */
    if(response_.file_len() > 0  && response_.file()) 
    {
        iov_[1].iov_base = response_.file();
        iov_[1].iov_len = response_.file_len();
        iov_cnt_ = 2;
    }

    return true;
}

bool HttpConn::process_read()
{
    int errno_;
    ssize_t len = -1;
    do {
        len = read_buffer_.read_fd(fd_, &errno_);
        if (len <= 0)
            break;
    } while (is_et_);

    if(len <= 0 && errno_ != EAGAIN)
    {
        epoll_->del_fd(this->get_fd());
        this->close_();
        return false;
    }

    if(process())
        epoll_->mod_fd(this->get_fd(), conn_event_ | EPOLLOUT);
    else
        epoll_->mod_fd(this->get_fd(), conn_event_ | EPOLLIN);
   
    return true;
}

bool HttpConn::process_write()
{
    int write_errno = 0;
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iov_cnt_);
        if(len <= 0) 
        {
            write_errno = errno;
            break;
        }
        if(iov_[0].iov_len + iov_[1].iov_len  == 0) 
            break;
        else if(static_cast<size_t>(len) > iov_[0].iov_len) 
        {
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);

            if(iov_[0].iov_len) 
            {
                write_buffer_.retrieve_all();
                iov_[0].iov_len = 0;
            }
        }
        else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len; 
            iov_[0].iov_len -= len; 
            write_buffer_.retrieve(len);
        }
    } while(is_et_ || write_bytes() > 10240);


    if(write_bytes() == 0) 
    {
        /* 传输完成 */
        if(request_.is_keep_alive()) 
        {
            if(process())
                epoll_->mod_fd(this->get_fd(), conn_event_ | EPOLLOUT);
            else
                epoll_->mod_fd(this->get_fd(), conn_event_ | EPOLLIN);
            
            return true;
        }
    }
    else if(len < 0) 
    {
        if(write_errno == EAGAIN) 
        {
            /* 继续传输 */
            epoll_->mod_fd(this->get_fd(), conn_event_ | EPOLLOUT);
            return true;
        }
    }
    close_();
}
