#include "config.h"

Config::Config()
{
    // database
    string db_user = "root";
    string db_pwd = "123456";
    string db_name = "webserver";

    potr = 8808; 
    sql_num = 8;                        // 数据库连接池数量,默认8
    thread_num = 8;                     // 线程池内的线程数量,默认8 
    open_log = 1;                       // 关闭日志,默认不关闭   
    actor_model = 0;                    // 并发模型
    max_requests = 10000;               // 最大连接数

    open_log = true;
    log_level = 1;
    log_queue_capacity = 1024;
}

void Config::parse_arg(int argc, char*argv[])
{
    //todo
}