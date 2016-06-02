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
#include "net.h"
#include "exit.h"
#include "server.h"
#include "global.h"
#include "logger.h"
#include "handler.h"
#include "database.h"
#include "envirment.h"
#include "login.h"

static int listen_fd = -1;
static int s_session = 0;
extern bool g_exit_flag;

static void s_clear_server(void * arg)
{

}

void fl_server_start()
{
	fl_add_main_exit_handle(s_clear_server);

	//step 1 start db server
	//fl_start_db_server

	//step 2 start game logic server
	//fl_start_game_logic_server

	//step 3 start gate server
	//fl_start_gate_server

	//step 4 start login server
	fl_start_login_server();

}

void fl_server_main_loop()
{

}  //server main loop end




