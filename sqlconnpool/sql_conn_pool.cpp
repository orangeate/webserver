#include "sql_conn_pool.h"

Connection_pool::Connection_pool()
{

}

Connection_pool *Connection_pool::GetInstance()
{
	static Connection_pool connPool;
	return &connPool;
}

/* 初始化，创建连接池 */
void Connection_pool::init(string url, int port, string user, 
    string pwd, string db_name, int max_conn)
{
    this->url_ = url;
    this->port_ = port;
    this->user_ = user;
	this->password_ = pwd;
	this->db_name_ = db_name;

    lock_.lock();
    MYSQL *con = NULL;
    con = mysql_init(con);

    for (int i = 0; i < max_conn; i++)
	{
		MYSQL *con = NULL;
		con = mysql_init(con);

		if (con == NULL)
		{
			cout << "Error:" << mysql_error(con);
			exit(1);
		}
		con = mysql_real_connect(con, url_.c_str(), user_.c_str(), password_.c_str(), db_name_.c_str(), port_, NULL, 0);

		if (con == NULL)
		{
			cout << "Error: " << mysql_error(con);
			exit(1);
		}
		conn_list_.push_back(con);
		++free_conn_nums_;
	}
}

/* 获取数据库连接 */
MYSQL *Connection_pool::get_mysql_connection()
{
	MYSQL *con = NULL;

	if (conn_list_.size() == 0)
		return NULL;

	sem_.wait();
	
	lock_.lock();

	con = conn_list_.front();
	conn_list_.pop_front();

	--free_conn_nums_;
	++used_conn_nums_;

	lock_.unlock();
	return con;
}

/* 释放数据库连接 */
bool Connection_pool::release_connection(MYSQL *con)
{
    if (con == nullptr)
		return false;
    
    lock_.lock();

	conn_list_.push_back(con);
	free_conn_nums_++;
	used_conn_nums_--;

	lock_.unlock();

	sem_.post();
	return true;
}

//销毁数据库连接池
void Connection_pool::destroy_pool()
{
	lock_.lock();
	if (conn_list_.size() > 0)
	{
		for (auto it = conn_list_.begin(); it != conn_list_.end(); ++it)
		{
			MYSQL *con = *it;
			mysql_close(con);
		}

		used_conn_nums_ = 0;
		free_conn_nums_ = 0;
		conn_list_.clear();

		lock_.unlock();
	}
	lock_.unlock();
}

/* 当前连接个数 */
int Connection_pool::cur_nums()
{
	return this->used_conn_nums_;
}

ConnectionRAII::ConnectionRAII(MYSQL **SQL, Connection_pool *SQL_poll)
{
	*SQL = SQL_poll->get_mysql_connection();
	
	conRAII = *SQL;
	poolRAII = SQL_poll;
}

ConnectionRAII::~ConnectionRAII()
{
	poolRAII->release_connection(conRAII);
}