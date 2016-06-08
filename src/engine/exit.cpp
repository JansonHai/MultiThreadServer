#include <vector>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "exit.h"
#include "global.h"

struct fl_exit_handle
{
	void (*func)(void * arg);
	void * args;
};

extern bool g_exit_flag;
static std::vector<struct fl_exit_handle *> main_exit_handles;
static std::vector<struct fl_exit_handle *> child_exit_handles;

void fl_add_main_exit_handle(void (*func)(void * args),void * args, size_t size)
{
	void * _arg = NULL;
	if (NULL != args)
	{
		_arg = malloc(size);
		memcpy(_arg,args,size);
	}
	struct fl_exit_handle * handle = new struct fl_exit_handle();
	memset(handle,0,sizeof(struct fl_exit_handle));
	handle->func = func;
	handle->args = _arg;
	main_exit_handles.push_back(handle);
}

void fl_main_exit()
{
	g_exit_flag = true;
	fl_exit_log("doing clear,please wait...");
	int i=0;
	struct fl_exit_handle * handle;
	for (i=main_exit_handles.size()-1;i>=0;--i)
	{
		handle = main_exit_handles[i];
		handle->func(handle->args);
		if (NULL != handle->args)
		{
			free(handle->args);
		}
		free(handle);//delete handle;
	}
	main_exit_handles.clear();
	usleep(1000000 * 10);  //sleep 10 second wait other process clear handle
	kill(0, SIGTERM);
	exit(0);
}

void fl_add_child_exit_handle(void (*func)(void * args),void * args, size_t size)
{

	void * _arg = NULL;
	if (NULL != args)
	{
		_arg = malloc(size);
		memcpy(_arg,args,size);
	}
	struct fl_exit_handle * handle = (struct fl_exit_handle *)malloc(sizeof(struct fl_exit_handle));  //new struct fl_exit_handle();
	memset(handle,0,sizeof(struct fl_exit_handle));
	handle->func = func;
	handle->args = _arg;
	child_exit_handles.push_back(handle);
}

void fl_child_exit()
{
	int i=0;
	struct fl_exit_handle * handle;
	for (i=child_exit_handles.size();i>=0;--i)
	{
		handle = child_exit_handles[i];
		handle->func(handle->args);
		if (NULL != handle->args)
		{
			free(handle->args);
		}
		free(handle);//delete child_exit_handles[i];
	}
	child_exit_handles.clear();
	_exit(0);
}

void fl_exit_log(const char * msg)
{
	int len = strlen(msg);
	if ('\n' == msg[len-1])
	{
		fprintf(stdout,"exit clear msg : %s",msg);
	}
	else
	{
		fprintf(stdout,"exit clear msg : %s\n",msg);
	}

}
