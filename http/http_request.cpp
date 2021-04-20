#include "http_request.h"

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML{
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture", };

void HttpRequest::init()
{
    method_ = path_ = version_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
}

std::string HttpRequest::path() const
{
    return path_;
}

std::string& HttpRequest::path()
{
    return path_;
}

std::string HttpRequest::method() const 
{
    return method_;
}

std::string HttpRequest::version() const 
{
    return version_;
}

bool HttpRequest::is_keep_alive() const 
{
    if(header_.find("Connection") != header_.end()) 
    {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

bool HttpRequest::parse_request_line(const std::string& line) {
    // GET / HTTP/1.1
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch sub_match;

    if(std::regex_match(line, sub_match, patten)) 
    {   
        method_ = sub_match[1];     // 请求方法
        path_ = sub_match[2];       // 请求Url
        version_ = sub_match[3];    // 协议及版本
        state_ = HEADERS;
        return true;
    }

    return false;
}

void HttpRequest::parse_header(const std::string& line)
{
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch sub_match;

    if(std::regex_match(line, sub_match, patten)) 
        header_[sub_match[1]] = sub_match[2];
    else
        state_ = BODY;

    parse_path();
}

void HttpRequest::parse_body(const std::string& line)
{
    body_ = line;
    parse_post();
    state_ = FINISH;
}

void HttpRequest::parse_path() 
{
    if(path_ == "/")
        path_ = "/index.html"; 
    else
    {
        for(auto &item: DEFAULT_HTML) {
            if(item == path_) {
                path_ += ".html";
                break;
            }
        }
    }
}

void HttpRequest::parse_post()
{
    if(method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") 
    {
        // 解析表单信息
        parse_from_urlencoded();

        // todo:
    }  
}

void HttpRequest::parse_from_urlencoded() 
{
    if(body_.size() == 0) 
        return; 

    std::string key, value;

    int num = 0;
    int size = body_.size();
    
    int i = 0, j = 0;
    for(; i < size; i++) 
    {
        char ch = body_[i];
        switch (ch) {
        case '=':
            key = body_.substr(j, i - j);
            j = i + 1;
            break;
        case '+':
            body_[i] = ' ';
            break;
        case '%':
            num = conver_hex(body_[i + 1]) * 16 + conver_hex(body_[i + 2]);
            body_[i + 2] = num % 10 + '0';
            body_[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = body_.substr(j, i - j);
            j = i + 1;
            post_[key] = value;
            break;
        default:
            break;
        }
    }

    assert(j <= i);

    if(post_.count(key) == 0 && j < i) 
    {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

bool HttpRequest::parse(Buffer& buff)
{
    const char CRLF[] = "\r\n";

    if(buff.readable_bytes() <= 0) 
        return false;

    // 有限状态机
    while(buff.readable_bytes() && state_ != FINISH) 
    {
        // 获取一行数据，根据\r\n为结束标志
        const char* line_end = std::search(buff.begin_read_const(), buff.begin_write_const(), CRLF, CRLF + 2);
        
        std::string line(buff.begin_read_const(), line_end);

        switch(state_)
        {
        case REQUEST_LINE:
            if(!parse_request_line(line)) 
                return false;
            break;    
        case HEADERS:
            parse_header(line);
            if(buff.readable_bytes() <= 2)
                state_ = FINISH;
            break;
        case BODY:
            parse_body(line);
            break;
        default:
            break;
        }
        if(line_end == buff.begin_write_const()) 
            break;

        buff.retrieve_until(line_end + 2);
    }
    return true;
}

int HttpRequest::conver_hex(char ch) 
{
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}