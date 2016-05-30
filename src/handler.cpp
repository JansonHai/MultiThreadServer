#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <mysql/mysql.h>
#include "global.h"
#include "logger.h"
#include "envirment.h"
#include "handler.h"
#include "exit.h"

static struct handle_context context;
extern MYSQL * g_mysql_conn;

static void s_clear_handle(void * arg)
{
	close(context.client.socketfd);
	fl_logger_close();
	if (NULL != context.mysql_conn)
	{
		mysql_close(context.mysql_conn);
	}
	mysql_library_end();
}

void fl_start_child_handle(struct handle_info client)
{

	fl_add_child_exit_handle(s_clear_handle);

//	fl_debug_log("process %d: mysql step -2", getpid());

//	mysql_library_init(0,NULL,NULL);

//	fl_debug_log("process %d: mysql step -1", getpid());

	//init context info
	context.client = client;
	context.mysql_conn = g_mysql_conn;

//	fl_debug_log("process %d: mysql step 0", getpid());

//	if (NULL == context.mysql_conn)
//	{
//		fl_debug_log("Process %d: Mysql init failed..\n",getpid());
//		fl_child_exit();
//	}

	fl_debug_log("process %d: mysql step 1", getpid());

	fl_start_child_logic(&context);

	fl_child_exit();
}

void fl_start_child_logic(struct handle_context * context)
{
//	fl_log(0,"Process %d : Accept client from %s : %d\n", getpid(), context->client.ip, context->client.port);

	char buf[4096];

	if (NULL == mysql_real_connect(context->mysql_conn, "127.0.0.1", "root", "", "test", 3306, NULL, 0))
	{
		snprintf(buf,sizeof(buf),"Server Error!Connect Mysql Failed...");
		send(context->client.socketfd,buf,strlen(buf)+1,0);
		fl_child_exit();
	}

	fl_debug_log("process %d: mysql step 2", getpid());

	char sql[4096];
//	snprintf(sql,sizeof(sql),"set names utf8");
//	mysql_real_query(context->mysql_conn, sql, strlen(sql));
	mysql_set_character_set(context->mysql_conn,"utf8");

	fl_debug_log("process %d: mysql step 3", getpid());

	snprintf(sql,sizeof(sql),"select * from `account`");
	if (-1 == mysql_real_query(context->mysql_conn, sql, strlen(sql)))
	{
		snprintf(buf,sizeof(buf),"Server Error!Mysql Query Failed...");
		send(context->client.socketfd,buf,strlen(buf)+1,0);
		fl_child_exit();
	}

	fl_debug_log("process %d: mysql step 4", getpid());

	MYSQL_RES * result = mysql_use_result(context->mysql_conn);

	MYSQL_ROW row = mysql_fetch_row(result);

	snprintf(buf,sizeof(buf),"Name:%s , Password:%s",row[1],row[2]);
	send(context->client.socketfd,buf,strlen(buf)+1,0);

	fl_debug_log("process %d: mysql step 5", getpid());

	mysql_free_result(result);

	fl_debug_log("process %d: mysql step 6", getpid());

//	int len = recv(context->client.socketfd,buf,sizeof(buf),0);
//	if (-1 != len)
//	{
//
//	}
//	sprintf(buf,"I receive your message,thank you!");
//	send(context->client.socketfd,buf,strlen(buf)+1,0);
}

