#include "webserver.h"

WebServer::WebServer(const Config& config) 
    : epoll_(Epoll::GetInstance()), 
    timer_(new HeapTimer()),
    p_thread_pool(new ThreadPool<HttpConn>(config.thread_num, config.max_requests))
{
    is_close_ = false;
    

    root_dir_ = getcwd(nullptr, 256);
    assert(root_dir_.size() != 0);
    root_dir_ += "/data/";

    HttpConn::user_count = 0;
    HttpConn::root_dir_ = root_dir_;

    init(config.potr, 3, config.timeout_ms,
    3306, config.db_user, config.db_pwd, config.db_name,
    config.sql_num, config.thread_num, 
    config.open_log, config.log_level, config.log_queue_capacity);
}

WebServer::~WebServer(){}

void WebServer::init(int port , int actor_model, int timeout_ms,
              int db_port, string db_user, string db_pwd, string db_name,
              int sql_num, int thread_num, 
              bool open_log, int log_level, int log_queue_capacity)
{
    this->port_ = port;
    this->actor_model_ = actor_model;
    this->thread_num_ = thread_num;
    
    // 初始化事件的模式
    init_event_mode(actor_model);
    
    // 日志
    if(open_log)
    {
        Log::GetInstance()->init(log_level, "./log", ".log", log_queue_capacity);

        LOG_INFO("========== Server init ==========");
        LOG_INFO("Port:%d", port_);
        LOG_INFO("Listen Mode: %s, OpenConn Mode: %s", (listen_event_ & EPOLLET ? "ET": "LT"), (conn_event_ & EPOLLET ? "ET": "LT"));
        LOG_INFO("log_sys level: %d", log_level);
        LOG_INFO("src_dir: %s", HttpConn::root_dir_.c_str());
    }

    // 初始化socket
    if(!init_socket())
    {   
        LOG_ERROR("========== Server stri error!==========");
        is_close_ = true;
    }
}

void WebServer::init_event_mode(int actor_model)
{
    listen_event_ = EPOLLRDHUP;
    conn_event_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (actor_model)
    {
    case 0:
        break;
    case 1:
        conn_event_ |= EPOLLET; 
        break;
    case 2:
        listen_event_ |= EPOLLET; 
        break;
    case 3:
        listen_event_ |= EPOLLET;  
        conn_event_ |= EPOLLET;    
        break;
    default:
        listen_event_ |= EPOLLET;     
        conn_event_ |= EPOLLET;  
        break;
    }
    HttpConn::is_et_ = (conn_event_ & EPOLLET);
}

bool WebServer::init_socket()
{
    int ret;
    struct sockaddr_in addr;
    if(port_ > 65535 || port_ < 1024)
    {
        LOG_ERROR("Port:%d error!",  port_);
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd_ < 0)
    {
        LOG_ERROR("Create socket error!", port_);
        return false;
    }

    int optval = 1;
    /* 端口复用 */
    ret = setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) 
    {
        LOG_ERROR("set socket setsockopt error !");
        close(listenfd_);
        return false;
    }

    ret = bind(listenfd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) 
    {
        LOG_ERROR("Bind Port:%d error!", port_);
        close(listenfd_);
        return false;
    }

    ret = listen(listenfd_, 6);
    if(ret < 0) 
    {
        LOG_ERROR("Listen port:%d error!", port_);
        close(listenfd_);
        return false;
    }

    ret = epoll_->add_fd(listenfd_,  listen_event_ | EPOLLIN);
    if(ret == 0) 
    {
        LOG_ERROR("Add listen error!");
        close(listenfd_);
        return false;
    }
    
    set_nonblock(listenfd_);
    LOG_INFO("Server port:%d", port_);
    return true;
}

static int set_nonblock(int fd) 
{
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

void WebServer::run() 
{
    int time_ms = -1;
    if(!is_close_) 
        { LOG_INFO("========== Server start =========="); }

    while (!is_close_)
    {
        if(timeout_ms_ > 0) 
            time_ms = timer_->get_next_tick();

        int event_nums = epoll_->wait(time_ms);  
        
        for(int i = 0; i < event_nums; i++)
        {
            /* 处理事件 */
            int sockfd = epoll_->get_event_fd(i);
            uint32_t events = epoll_->get_events(i);

            if(sockfd == listenfd_) 
            {
                deal_listen();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) 
            {
                assert(users_.count(sockfd) > 0);
                close_conn(&users_[sockfd]);
            }
            else if(events & EPOLLIN) 
            {
                assert(users_.count(sockfd) > 0);
                deal_read(&users_[sockfd]);
            }
            else if(events & EPOLLOUT) 
            {
                assert(users_.count(sockfd) > 0);
                deal_write(&users_[sockfd]);
            }
            else
            {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void WebServer::deal_listen()
{
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    do {
        int connfd = accept(listenfd_, (struct sockaddr *)&client_address, &client_addrlength);
        if(connfd <= 0) 
            return;
        // todo: 客户端连接上限
        add_client(connfd, client_address);              //添加客户端
    } while(listen_event_ & EPOLLET);
}

void WebServer::deal_read(HttpConn* client) 
{
    assert(client);
    extent_time(client);
    p_thread_pool->append(client, 0);                   // 添加到消息队列
}

void WebServer::deal_write(HttpConn* client) 
{
    assert(client);
    extent_time(client);
    p_thread_pool->append(client, 1);                   // 添加到消息队列
}

void WebServer::add_client(int fd, sockaddr_in addr) 
{
    assert(fd > 0);
    users_[fd].init(fd, conn_event_ ,addr);

    if(timeout_ms_ > 0) 
    {
        timer_->push(fd, timeout_ms_, std::bind(&WebServer::close_conn, this, &users_[fd]));
    }


    epoll_->add_fd(fd, conn_event_ | EPOLLIN);          // 监听读事件
    set_nonblock(fd);                                   // 设置非阻塞

    LOG_INFO("Client[%d] in!", users_[fd].get_fd());
}

void WebServer::close_conn(HttpConn* client)
{
    assert(client);
    LOG_INFO("Client[%d] quit!", client->get_fd());

    epoll_->del_fd(client->get_fd());
    client->close_();
}

void WebServer::extent_time(HttpConn* client) 
{
    assert(client);
    if(timeout_ms_ > 0) { timer_->adjust(client->get_fd(), timeout_ms_); }
}

