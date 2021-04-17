#include "buffer.h"

Buffer::Buffer(int size) : buffer_(size), read_pos_(0), write_pos_(0) 
{

}
/*
        
        begin_ptr              begin_read                     begin_write     
        ---------------------------------------------------------------------------------------
       |          |          | read_pos_ |         |         | write_pos_ |         |         |
        ---------------------------------------------------------------------------------------
       |- prependable_bytes -|           |-  readable_bytes -|            |-  writable_bytes -|
*/

char* Buffer::begin_ptr()
{
    return &*buffer_.begin();
}

const char* Buffer::begin_ptr() const
{
    return &*buffer_.begin();
}

char* Buffer::begin_write()
{
    return begin_ptr() + write_pos_;
}

const char* Buffer::begin_write_const() const
{
    return begin_ptr() + write_pos_;
}

const char* Buffer::begin_read_const() const
{
    return begin_ptr() + read_pos_;
}

size_t Buffer::writable_bytes() const
{
    return buffer_.size() - write_pos_;
}

size_t Buffer::readable_bytes() const
{
    return write_pos_ - read_pos_;
}

size_t Buffer::prependable_bytes() const 
{
    return read_pos_;
}

void Buffer::retrieve_all() 
{
    bzero(&buffer_[0], buffer_.size());
    read_pos_ = 0;
    write_pos_ = 0;
}

std::string Buffer::retrieve_all_to_str() {
    std::string str(begin_read_const(), readable_bytes());
    retrieve_all();
    return str;
}

void Buffer::make_space(size_t len)
{
    if(writable_bytes() + prependable_bytes() < len) 
    {
        buffer_.resize(write_pos_ + len + 1);
    } 
    else 
    {
        size_t readable = readable_bytes();
        std::copy(begin_ptr() + read_pos_, begin_ptr() + write_pos_, begin_ptr());
        read_pos_ = 0;
        write_pos_ = read_pos_ + readable;
        assert(readable == readable_bytes());
    }
}

void Buffer::ensure_writeable(size_t len)
{
    if(writable_bytes() < len)
        make_space(len);
    assert(writable_bytes() >= len);
}

void Buffer::has_written(size_t len)
{
    write_pos_ += len;
}

void Buffer::retrieve(size_t len) 
{
    assert(len <= readable_bytes());
    read_pos_ += len;
}

void Buffer::retrieve_until(const char* end) 
{
    assert(begin_read_const() <= end );
    retrieve(end - begin_read_const());
}

void Buffer::append(const char* str, size_t len)
{
    assert(str);
    ensure_writeable(len);
    std::copy(str, str + len, begin_write());
    has_written(len);
}

void Buffer::append(const void* data, size_t len) 
{
    assert(data);
    append(static_cast<const char*>(data), len);
}

void Buffer::append(const std::string& str) 
{
    append(str.data(), str.size());
}

void Buffer::append(const Buffer& buff) 
{
    append(buff.begin_read_const(), buff.readable_bytes());
}

ssize_t Buffer::read_fd(int fd, int* save_err)
{
    char buff[65535];

    struct iovec iov[2];
    const size_t writable = writable_bytes();


    // 分散读， 保证全部读完
    iov[0].iov_base = begin_ptr() + write_pos_; // 读位置
    iov[0].iov_len = writable;                  // 可读入大小

    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if(len < 0)
        *save_err = errno;
    else if(static_cast<size_t>(len) <= writable) // 全部写入
        write_pos_ += len;
    else
    {                                             // 将buff中的数据添加到buffer_
        write_pos_ = buffer_.size();
        append(buff, len - writable);
    }

    return len;
}

















