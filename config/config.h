#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <unistd.h>
using namespace  std;

class Config
{
public:
    Config();
    ~Config(){};

    void parse_arg(int argc, char*argv[]);

    int potr;
    int sql_num;
    int thread_num;
    int actor_model;
    int max_requests;

    // log
    bool open_log;
    int log_level;
    int log_queue_capacity;

    // db
    string db_user;
    string db_pwd;
    string db_name;
};
#endif //CONFIG_H