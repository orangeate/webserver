#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>         
#include <assert.h>
#include <sys/stat.h>   
#include <assert.h>

#include "../lock/lock.h"
#include "block_queue.h"
#include "../buffer/buffer.h"

class Log 
{
public:
    void init(int level  = 1, const char* path = "./log",  const char* suffix =".log", int capacity = 1024);

    static Log* GetInstance();
    static void flush_log_thread();

    void write(int level, const char *format,...);
    void flush();

    int get_level();
    void set_level(int level);
    bool is_open() { return is_open_; }

private:
    Log();
    ~Log();

    void append_log_level_title_(int level);
    void async_write();
    
private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path_;
    const char* suffix_;

    int MAX_LINES_;

    int line_count_;
    int day_;

    bool is_open_;

    Buffer buff_;
    int level_;
    bool is_async_;                         // 异步

    FILE* fp_;
    std::unique_ptr<BlockDeque<std::string>> deque_; 
    std::unique_ptr<std::thread> write_thread_;

    Mutex mutex_;
};

#define LOG_BASE(level, format, ...) \
    {\
        do {\
            Log* log = Log::GetInstance();\
            if (log->is_open() && log->get_level() <= level) {\
                log->write(level, format, ##__VA_ARGS__); \
                log->flush();\
            }\
            } while(0);\
    }

#define LOG_DEBUG(format, ...) {do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);}
#define LOG_INFO(format, ...) {do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);}
#define LOG_WARN(format, ...) {do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);}
#define LOG_ERROR(format, ...) {do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);}

#endif //LOG_H