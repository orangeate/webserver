#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <sys/epoll.h>
#include <iostream>

#include "../threadpool/thread_pool.h"
#include "../http/http_conn.h"
#include "../epoll/epoll.h"
#include "../config/config.h"
#include "../log/log.h"

using namespace std;

static int set_nonblock(int fd); //设置非阻塞
static const int MAX_FD = 65536;  //最大文件描述符个数

class WebServer
{
public:
    // 端口号 模式
    // 数据库
    // 连接池数量 线程池容量 日志
    WebServer(const Config& config);
    ~WebServer();

    void run();

private:
    void init(int port , int actor_model,
              int db_port, string db_user, string db_pwd, string db_name,
              int sql_num, int thread_num, 
              bool open_log, int log_level, int log_queue_capacity);

    bool init_socket();

    bool init_thread_pool();

    void init_event_mode(int actor_model);
    
    void add_client(int fd, sockaddr_in addr);

    void deal_listen();
    void deal_read(HttpConn* client);
    void deal_write(HttpConn* client);

    void send_error(int fd, const char*info);
    void close_conn(HttpConn* client);

private:
    int port_;              // 端口号
    int listenfd_;          // 监听文件描述符
    bool is_close_;         // 是否关闭
    int actor_model_;       // 并发模式   
    int thread_num_;        // 线程池数量
    string root_dir_;       // 资源根目录 

    uint32_t listen_event_; // 监听的文件描述符事件
    uint32_t conn_event_;   // 连接的文件描述符事件

    Epoll* epoll_;
    std::unique_ptr<ThreadPool<HttpConn>> p_thread_pool;
    std::unordered_map<int, HttpConn> users_;               //  连接的信息 fd -> httpconn

};