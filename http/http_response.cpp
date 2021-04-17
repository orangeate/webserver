#include "http_response.h"

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = 
{
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const std::unordered_map<int, std::string> HttpResponse::CODE_PATH = 
{
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS = 
{
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

HttpResponse::HttpResponse() 
{
    code_ = -1;
    path_ = src_dir_ = "";
    is_keep_alive = false;
    mm_file_ = nullptr; 
    mm_file_stat_ = { 0 };
};

HttpResponse::~HttpResponse() 
{
    unmap_file();
}

void HttpResponse::init(const std::string& src_dir, std::string& path, bool is_keep_alive, int code)
{
    assert(src_dir != "");
    
    if(mm_file_) 
        unmap_file();

    code_ = code;
    is_keep_alive = is_keep_alive;
    path_ = path;
    src_dir_ = src_dir;
    mm_file_ = nullptr; 
    mm_file_stat_ = { 0 };
}

char* HttpResponse::file() 
{
    return mm_file_;
}

size_t HttpResponse::file_len() const
{
    return mm_file_stat_.st_size;
}

void HttpResponse::unmap_file() 
{
    if(mm_file_) 
    {
        munmap(mm_file_, mm_file_stat_.st_size);
        mm_file_ = nullptr;
    }
}

std::string HttpResponse::get_file_type() 
{
    /* 判断文件类型 */
    std::string::size_type idx = path_.find_last_of('.');
    if(idx == std::string::npos) 
    {
        return "text/plain";
    }
    std::string suffix = path_.substr(idx);
    if(SUFFIX_TYPE.count(suffix) == 1) 
    {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

void HttpResponse::error_html()
{
    if(CODE_PATH.count(code_) == 1)
    {
        path_ = CODE_PATH.find(code_)->second;
        stat((src_dir_ + path_).data(), &mm_file_stat_);
    }
}

void HttpResponse::error_content(Buffer& buff, std::string message) 
{
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }
    body += std::to_string(code_) + " : " + status  + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>WebServer</em></body></html>";

    buff.append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buff.append(body);
}

void HttpResponse::add_state_line(Buffer& buff) 
{
    std::string status;
    if(CODE_STATUS.count(code_) == 1) 
    {
        status = CODE_STATUS.find(code_)->second;
    }
    else 
    {
        code_ = 400;
        status = CODE_STATUS.find(400)->second;
    }
    buff.append("HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n");
}

void HttpResponse::add_header(Buffer& buff) 
{
    buff.append("Connection: ");
    if(is_keep_alive) {
        buff.append("keep-alive\r\n");
        buff.append("keep-alive: max=6, timeout=120\r\n");
    } 
    else
    {
        buff.append("close\r\n");
    }
    buff.append("Content-type: " + get_file_type() + "\r\n");
}

void HttpResponse::add_content(Buffer& buff) {
    int srcFd = open((src_dir_ + path_).data(), O_RDONLY);
    if(srcFd < 0) { 
        error_content(buff, "File NotFound!");
        return; 
    }

    /* 将文件映射到内存提高文件的访问速度 MAP_PRIVATE 建立一个写入时拷贝的私有映射*/
    int* mm_ret = (int*)mmap(0, mm_file_stat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if(*mm_ret == -1) 
    {
        error_content(buff, "File NotFound!");
        return; 
    }
    mm_file_ = (char*)mm_ret;
    close(srcFd);
    buff.append("Content-length: " + std::to_string(mm_file_stat_.st_size) + "\r\n\r\n");
}

void HttpResponse::make_response(Buffer& buff) 
{
    /* 判断请求的资源文件 */
    // index.html
    if(stat((src_dir_ + path_).data(), &mm_file_stat_) < 0 || S_ISDIR(mm_file_stat_.st_mode))
        code_ = 404;
    else if(!(mm_file_stat_.st_mode & S_IROTH))
        code_ = 403;
    else if(code_ == -1)
        code_ = 200; 

    error_html();

    add_state_line(buff);
    add_header(buff);
    add_content(buff);
}





