#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap
#include <unordered_map>
#include "../buffer/buffer.h"

class HttpResponse 
{
public:
    HttpResponse();
    ~HttpResponse();

    void init(const std::string& src_dir, std::string& path, bool is_keep_alive = false, int code = -1);

    void make_response(Buffer& buff);

    char* file();
    size_t file_len() const;
    void unmap_file();
    
    int code() const { return code_; }

private:
    void add_state_line(Buffer &buff);
    void add_header(Buffer &buff);
    void add_content(Buffer &buff);

    void error_html();
    void error_content(Buffer& buff, std::string message);

    std::string get_file_type();

private:

    int code_;                      // 响应状态码
    bool is_keep_alive;             // 是否保持连接

    std::string path_;              // 资源路径
    std::string src_dir_;           // 资源的目录
    
    char* mm_file_;                 // 文件内存映射的指针
    struct stat mm_file_stat_;      // 文件的状态信息

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;  // 后缀 - 类型
    static const std::unordered_map<int, std::string> CODE_STATUS;          // 状态码 - 描述 
    static const std::unordered_map<int, std::string> CODE_PATH;            // 状态码 - 路径
};

#endif //HTTP_RESPONSE_H