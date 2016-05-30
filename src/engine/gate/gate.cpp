#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "gate.h"
#include "logger.h"

static int s_gate_pid = 0;
static int s_gate_pipe[2];

static void s_start_net_gate_logic();
static void s_start_net_gate_loop();
static bool s_run_state = false;

bool fl_start_net_gate_server()
{
	int ret = pipe(s_gate_pipe);
	if (ret < 0)
	{
		fl_log(2,"start gate pipe failed,errno = %d\n", errno);
		return false;
	}

	s_gate_pid = fork();
	if (s_gate_pid < 0)
	{
		fl_log(2,"fork gate child process failed,errno = %d\n", errno);
		return false;
	}

	if (0 == s_gate_pid)
	{
		//child close write
		close(s_gate_pipe[1]);
		s_run_state = true;
		s_start_net_gate_logic();
		_exit(0);
	}
	else
	{
		//parent close read
		fl_log(2,"fork gate child process successfully,child pid = %d\n", s_gate_pid);
		close(s_gate_pipe[0]);
	}
	return true;
}

static void s_start_net_gate_logic()
{

	s_start_net_gate_loop();
}

static void s_start_net_gate_loop()
{
	while (s_run_state)
	{
		sleep(1);
	}
}


