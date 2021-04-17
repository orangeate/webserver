#ifndef BUFFER_H
#define BUFFER_H
#include <cstring> 
#include <iostream>
#include <unistd.h> 
#include <sys/uio.h> 
#include <vector>
#include <atomic>
#include <assert.h>

class Buffer
{
public:
    Buffer(int size = 1024);
    ~Buffer() = default;

    void ensure_writeable(size_t len);
    void has_written(size_t len);

    void retrieve(size_t len);
    void retrieve_until(const char* end);

    void retrieve_all();
    std::string retrieve_all_to_str();

    const char* begin_read_const() const;
    const char* begin_write_const() const;
    char* begin_write();

    void append(const std::string& str);
    void append(const char* str, size_t len);
    void append(const void* data, size_t len);
    void append(const Buffer& buff);

    ssize_t read_fd(int fd, int* Errno);
    ssize_t write_fd(int fd, int* Errno);

    size_t writable_bytes() const;       
    size_t readable_bytes() const ;
    size_t prependable_bytes() const;

    
private:
   
    

private:
    char* begin_ptr();
    const char* begin_ptr() const;
    void make_space(size_t len);

    std::vector<char> buffer_;
    
    std::atomic<std::size_t> read_pos_;
    std::atomic<std::size_t> write_pos_;
};

#endif //BUFFER_H