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
    open_log = 1;                       // 关闭日志,默认不关闭   
    actor_model = 3;                    // 并发模型
    
    open_log = true;                    // 日志
    log_level = 1;                      // 日志级别
    log_queue_capacity = 1024;

    timeout_ms = 60000;                 // 连接超时时间 默认60s
}

void Config::parse_arg(int argc, char*argv[])
{
    //todo
}