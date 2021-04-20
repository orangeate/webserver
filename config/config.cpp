#include "config.h"

Config::Config()
{
    // database
    string db_user = "root";
    string db_pwd = "123456";
    string db_name = "webserver";

    sql_num = 8;                        // 数据库连接池数量,默认8
    max_requests = 10000;               // 最大连接数

    potr = 8808; 
    thread_num = 8;                     // 线程池内的线程数量,默认8   
    model = 3;                          // 并发模型
    
    open_log = 1;                       // 日志
    log_level = 1;                      // 日志级别
    log_queue_capacity = 1024;

    timeout_ms = 60000;                 // 连接超时时间 默认60s
}

void Config::parse_args(int argc, char*argv[])
{
    int opt;
    const char *opt_str = "p:m:l:";        // port : model : log_level
    while ((opt = getopt(argc, argv, opt_str)) != -1)
    {
        switch(opt)
        {
            case 'p':
            {
                potr = atoi(optarg);
                break;
            }
            case 'm':
            {
                model = atoi(optarg);
                break;
            }
            case 'l':
            {
                log_level = atoi(optarg);
                if(log_level == 0)
                    open_log = 0;
                break;
            }
        }
    }
}