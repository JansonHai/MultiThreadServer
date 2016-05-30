#include "../engine/database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "../engine/exit.h"
#include "envirment.h"
#include "logger.h"

static int MAX_SQL_CONNECTION = 256;
static pthread_mutex_t * s_sql_lock;
static bool * s_sql_flag;
static MYSQL * s_sql_conn;
static pthread_mutex_t s_get_mysql_lock;

static void s_exit_handle(void * args)
{
	for (int i=0;i<MAX_SQL_CONNECTION;++i)
	{
		pthread_mutex_destroy(&s_sql_lock[i]);
		mysql_close(&s_sql_conn[i]);
		s_sql_flag[i] = false;
	}
	free(s_sql_lock);
	free(s_sql_flag);
	free(s_sql_conn);
	pthread_mutex_destroy(&s_get_mysql_lock);
	mysql_library_end();
}

void fl_database_init()
{
	mysql_library_init(0,NULL,NULL);
	MAX_SQL_CONNECTION = fl_getenv("max_database_connection", 256);
	s_sql_lock = (pthread_mutex_t *)malloc(MAX_SQL_CONNECTION * sizeof(pthread_mutex_t));
	s_sql_flag = (bool *)malloc(MAX_SQL_CONNECTION * sizeof(bool));
	s_sql_conn = (MYSQL *)malloc(MAX_SQL_CONNECTION * sizeof(MYSQL));
	for (int i=0;i<MAX_SQL_CONNECTION;++i)
	{
		pthread_mutex_init(&s_sql_lock[i],NULL);
		s_sql_flag[i] = false;
	}
	pthread_mutex_init(&s_get_mysql_lock,NULL);
	fl_add_main_exit_handle(s_exit_handle);
	fl_log(0,"init database successfully\n");
}

fl_mysql_conn * fl_database_get_conn()
{
	fl_mysql_conn * mysql_conn;

	mysql_conn = new struct fl_mysql_conn();
	mysql_conn->id = MAX_SQL_CONNECTION + 1;

	pthread_mutex_lock(&s_get_mysql_lock);
	for (int i=0;i<MAX_SQL_CONNECTION;++i)
	{
		if (0 == pthread_mutex_trylock(&s_sql_lock[i]))
		{
			s_sql_flag[i] = true;
			mysql_conn->id = i;
			mysql_conn->mysql = &s_sql_conn[i];
			break;
		}
	}
	pthread_mutex_unlock(&s_get_mysql_lock);

	if (NULL != mysql_conn->mysql)
	{
		return mysql_conn;
	}

	int rd = rand() % MAX_SQL_CONNECTION;
	pthread_mutex_lock(&s_sql_lock[rd]);
	s_sql_flag[rd] = true;
	mysql_conn->id = rd;
	mysql_conn->mysql = &s_sql_conn[rd];

	return mysql_conn;
}

void fl_database_reuse_conn(fl_mysql_conn * conn)
{
	if (NULL == conn) return;
	if (conn->id < 0 || conn->id >= MAX_SQL_CONNECTION) return;
	if (false == conn->isFreeResult)
	{
		fl_database_free_result(conn);
	}
	mysql_close(conn->mysql);
	s_sql_flag[conn->id] = false;
	pthread_mutex_unlock(&s_sql_lock[conn->id]);
	delete conn;
}

bool fl_database_connect(fl_mysql_conn * conn,const char * host,const char * user,const char * passwd,const char * db,unsigned int port)
{
	if (NULL == conn) return false;
	mysql_init(conn->mysql);
	if (NULL == mysql_real_connect(conn->mysql, host, user, passwd, db, port, NULL, 0))
	{
		return false;
	}
	return true;
}

bool fl_database_query(fl_mysql_conn * conn, const char * sql)
{
	if (NULL == conn) return false;
	int flag;
	flag = mysql_query(conn->mysql, sql);
	if (0 == flag)
	{
		conn->result = mysql_use_result(conn->mysql);
		conn->maxRowCount = mysql_num_rows(conn->result);
		conn->isFreeResult = false;
		conn->rowCount = 0;
		conn->row = NULL;
		return true;
	}
	else
	{
		conn->result = NULL;
		conn->isFreeResult = true;
		return false;
	}
	return false;
}

bool fl_database_real_query(fl_mysql_conn * conn, const char * sql, unsigned long length)
{
	if (NULL == conn) return false;
	int flag;
	flag = mysql_real_query(conn->mysql, sql, length);
	if (0 == flag)
	{
		conn->result = mysql_use_result(conn->mysql);
		conn->maxRowCount = mysql_num_rows(conn->result);
		conn->isFreeResult = false;
		conn->rowCount = 0;
		conn->row = NULL;
		return true;
	}
	else
	{
		conn->result = NULL;
		conn->isFreeResult = true;
		return false;
	}
	return false;
}

bool fl_database_fetch_row(fl_mysql_conn * conn)
{
	if (NULL != conn && NULL != conn->result && conn->rowCount < conn->maxRowCount)
	{
		++conn->rowCount;
		conn->row = mysql_fetch_row(conn->result);
		return true;
	}
	return false;
}

void fl_database_free_result(fl_mysql_conn * conn)
{
	if (NULL != conn && NULL != conn->result)
	{
		mysql_free_result(conn->result);
		conn->isFreeResult = true;
		conn->result = NULL;
	}
}

