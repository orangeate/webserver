#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>     
#include <mysql/mysql.h> 
#include "../buffer/buffer.h"

class HttpRequest 
{
public:

    enum PARSE_STATE {
        REQUEST_LINE,   // 解析请求首行
        HEADERS,        // 解析请求头
        BODY,           // 解析请求体
        FINISH,         // 解析完成
    };

    HttpRequest() {};
    ~HttpRequest() = default;

    bool parse(Buffer& buff);

    void init();

    std::string path() const;
    std::string& path();
    std::string method() const;
    std::string version() const;

    bool is_keep_alive() const;

private:

    bool parse_request_line(const std::string& line);       // 解析请求行
    void parse_header(const std::string& line);             // 解析请求头
    void parse_body(const std::string& line);
    
    void parse_path();
    void parse_post();
    void parse_from_urlencoded();

    int conver_hex(char ch);

private:

    PARSE_STATE state_;                                     // 解析的状态
    std::string method_;                                    // 请求方法
    std::string path_;                                      // 请求路径
    std::string version_;                                   // 协议版本
    std::string body_;                                      // 请求体

    std::unordered_map<std::string, std::string> header_;   // 请求头
    std::unordered_map<std::string, std::string> post_;     // 请求表单数据

    static const std::unordered_set<std::string> DEFAULT_HTML;  // 默认的网页

    static int ConverHex(char ch);  // 转换成十六进制
}; 

#endif //HTTP_REQUEST_H