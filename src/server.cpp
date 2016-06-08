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
#include <list>
#include <errno.h>
#include <sys/select.h>
#include <time.h>
#include <fcntl.h>
#include "exit.h"
#include "server.h"
#include "global.h"
#include "logger.h"
#include "handler.h"
#include "database.h"
#include "envirment.h"
#include "login.h"
#include "gate.h"

static bool s_connect_to_login_watch_dog();
static bool s_connect_to_net_gate_watch_dog();
static class fl_connection login_conn;
static class fl_connection gate_conn;

static void s_clear_server(void * arg)
{
	login_conn.Send("shutdown",strlen("shutdown"));
	gate_conn.Send("shutdown",strlen("shutdown"));
}

void fl_server_start()
{

	//step 1 start game logic server
	//fl_start_game_logic_server

	//step 2 start gate server
	fl_start_net_gate_server();
	sleep(1);

	//step 3 start login server
	fl_start_login_server();
	sleep(1);

	if (false == s_connect_to_net_gate_watch_dog())
	{
		fl_main_exit();
	}
	fl_log(0,"[Main] : connect to gate watchdog server successfully\n");

	if (false == s_connect_to_login_watch_dog())
	{
		fl_main_exit();
	}
	fl_log(0,"[Main] : connect to login watchdog server successfully\n");

	fl_add_main_exit_handle(s_clear_server);

}

void fl_server_main_loop()
{
	while (true)
	{
		sleep(2);
	}
}  //server main loop end


static bool s_connect_to_login_watch_dog()
{
	int server_fd = socket(AF_INET,SOCK_STREAM,0);
	if (-1 == server_fd)
	{
		fl_log(2,"[Main] : login server create connect socket failed, errno: %d\n", errno);
		return false;
	}

	struct sockaddr_in server_addr;

	const char * ip = fl_getenv("login_server_ip", "127.0.0.1");
	int port = fl_getenv("login_server_ctrl_port", 6601);

	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);

	if (-1 == connect(server_fd,(struct sockaddr*)&server_addr, sizeof(struct sockaddr)))
	{
		fl_log(2,"[Main] : connect to login server [%s:%d] failed, errno:%d\n", ip, port, errno);
		return false;
	}
	login_conn.SetIndex(0);
	login_conn.SetAddrInfo(&server_addr);
	login_conn.SetSocketInfo(server_fd, 1);
	return true;
}

static bool s_connect_to_net_gate_watch_dog()
{
	int server_fd = socket(AF_INET,SOCK_STREAM,0);
	if (-1 == server_fd)
	{
		fl_log(2,"[Main] : gate server create connect socket failed, errno: %d\n", errno);
		return false;
	}

	struct sockaddr_in server_addr;

	const char * ip = fl_getenv("gate_server_ip", "127.0.0.1");
	int port = fl_getenv("gate_server_ctrl_port", 6701);

	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);

	if (-1 == connect(server_fd,(struct sockaddr*)&server_addr, sizeof(struct sockaddr)))
	{
		fl_log(2,"[Main] : connect to gate server [%s:%d] failed, errno:%d\n", ip, port, errno);
		return false;
	}
	gate_conn.SetIndex(0);
	gate_conn.SetAddrInfo(&server_addr);
	gate_conn.SetSocketInfo(server_fd, 1);
	return true;
}




