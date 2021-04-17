#ifndef SQL_CONN_POOL_H
#define SQL_CONN_POOL_H
#include <error.h>
#include <iostream>
#include <string>
#include <list>
#include <stdlib.h>
#include <pthread.h>
#include <error.h>
#include <mysql/mysql.h>

#include "../lock/lock.h"

using namespace std;

// mysql连接池
class Connection_pool
{
public:
	MYSQL *get_mysql_connection();				 	 //获取数据库连接
	bool release_connection(MYSQL *conn); 			 //释放连接

	
	void destroy_pool();					 		 //销毁所有连接
	int cur_nums();					 				 //当前连接数

	static Connection_pool *GetInstance();
	void init(string url, int port, string user, string password, string database_name, int max_conn);
	
	Connection_pool();
	~Connection_pool();

private:
	unsigned int max_conn_nums_;
	unsigned int used_conn_nums_; 
	unsigned int free_conn_nums_;

private:
	Mutex lock_;
	list<MYSQL *> conn_list_;
	Semaphore sem_;

/* 数据库信息 */
private:
    int port_;		            //数据库端口号
	string url_;			    //主机地址
	string user_;		        //登陆数据库用户名
	string password_;	        //登陆数据库密码
	string db_name_;            //使用数据库名
};

class ConnectionRAII{
public:
	ConnectionRAII(MYSQL **con, Connection_pool *connPool);
	~ConnectionRAII();
	
private:
	MYSQL *conRAII;
	Connection_pool *poolRAII;
};

#endif //SQL_CONN_POOL_H