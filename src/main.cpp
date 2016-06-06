#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "exit.h"
#include "global.h"
#include "server.h"
#include "logger.h"
#include "envirment.h"

int main(int argc, char* argv[])
{
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


	fprintf(stdout,"main server start,pid = %d\n",getpid());
	fl_log(0,"\n\n===============================================================\n\n");
	fl_log(0,"main server start,pid = %d\n",getpid());

	fl_server_start();

	fl_server_main_loop();

	fl_main_exit();

    return 0;
}


