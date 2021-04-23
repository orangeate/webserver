# WebServer

学习Linux网络编程期间完成的Web服务器

- [x] 线程池 + IO复用技术（Epoll ET/LT） + 事件处理（Reactor）
- [x] HTTP请求解析（状态机+ 正则） 
- [x] 自动增长的缓冲区
- [x] 日志（单例+阻塞队列）
- [x] 定时器（堆）
- [ ] 数据库

```
.
├── buffer
│   ├── buffer.h
│   └── buffer.cpp
├── config
│   ├── config.h
│   └── config.cpp
├── data
│   ├── ...
│   └── ...
├── epoll
│   ├── epoll.h
│   └── epoll.cpp
├── http
│   ├── http_coon.h
│   ├── http_request.h
│   ├── http_response.h
│   ├── http_coon.cpp
│   ├── http_request.cpp
│   └── http_response.cpp
├── lock
│   ├── lock.h
│   └── lock.cpp
├── log          
│   ├── block_queue.h
│   ├── log.h
│   ├── block_queue.cpp
│   └── log.cpp
├── sqlconnpool
│   ├── sql_conn_pool.h
│   └── sql_conn_pool.cpp
├── threadpool
│   ├── thread_pool.h
│   └── thread_pool.cpp
├── time
│   ├── heap_timer.h
│   └── heap_timer.cpp
├── webserver
│   ├── webserver.h
│   └── webserver.cpp
└── main.cpp
```