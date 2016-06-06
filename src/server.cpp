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
#include <signal.h>
#include "exit.h"
#include "server.h"
#include "global.h"
#include "logger.h"
#include "handler.h"
#include "database.h"
#include "envirment.h"
#include "login.h"
#include "gate.h"

static void s_clear_server(void * arg)
{
	fl_stop_login_server();
	fl_stop_net_gate_server();
}

static void s_signal_handler(int s)
{
	if (s == SIGINT || s == SIGTERM)
	{
		fprintf(stdout,"\nget a signal Interrupt (Ctrl-C)\n");
		fl_log(9999,"get a signal Interrupt (Ctrl-C),The program will exit\n");
		fl_main_exit();
	}
}

void fl_server_start()
{
	fl_add_main_exit_handle(s_clear_server);

	//step 1 start game logic server
	//fl_start_game_logic_server

	//step 2 start gate server
	fl_start_net_gate_server();

	//step 3 start login server
	fl_start_login_server();

	signal(SIGINT,s_signal_handler);
}

void fl_server_main_loop()
{
	while (true)
	{
		sleep(2);
	}
}  //server main loop end




