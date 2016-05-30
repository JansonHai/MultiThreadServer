#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "global.h"
#include "exit.h"
#include "server.h"
#include "system/envirment.h"
#include "system/logger.h"

static void signal_handler(int s)
{
	if (s == SIGINT || s == SIGTERM)
	{
		fprintf(stdout,"\nget a signal Interrupt (Ctrl-C)\n");
		fl_log(9999,"get a signal Interrupt (Ctrl-C),The program will exit\n");
		fl_main_exit();
	}
}


int main(int argc, char* argv[])
{

	signal(SIGINT,signal_handler);

	if (argc < 2)
	{
		fprintf(stderr,"need a config file,please use %s [configName]\n",argv[0]);
		exit(0);
	}

	if (false == fl_load_config(argv[1]))
	{
		fprintf(stderr,"load config file %s failed, the program will exit\n",argv[1]);
		exit(0);
	}

	fprintf(stdout,"load config file %s successfully\n",argv[1]);

	srand(time(0));

	fl_logger_init();

	fprintf(stdout,"server start,pid = %d\n",getpid());
	fl_log(0,"\n\n===============================================================\n\n");
	fl_log(0,"server start,pid = %d\n",getpid());

	fl_server_start();

	fl_server_main_loop();

	fl_main_exit();

    return 0;
}


