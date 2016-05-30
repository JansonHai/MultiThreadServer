#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <stdarg.h>
#include <sys/stat.h>
#include "logger.h"
#include "envirment.h"
#include "exit.h"

static int loggerfd = -1;

static void s_clear_logger(void * arg)
{
	if (-1 != loggerfd)
	{
		fl_exit_log("close logger fd\n");
		close(loggerfd);
		loggerfd = -1;
	}
}

void fl_logger_init()
{
	if (-1 == loggerfd)
	{
		fl_add_main_exit_handle(s_clear_logger);
		const char * logPath = fl_getenv("log_path","./server.log");
		int len = strlen(logPath);
		const char *ch = logPath + len - 1;
		int index = len - 1;
		while (*ch != '/' && index > 0) --ch,--index;
		if (index > 0)
		{
			char path[1024];
			char buf[1024];
			strncpy(path,logPath,index + 1);
			path[index+2]=0;
			DIR * dir = opendir(path);
			if (NULL == dir)
			{
				fprintf(stdout,"mkdir -m 0644 -p %s\n",path);
				sprintf(buf,"mkdir -m 0644 -p %s",path);
				system(buf);
			}
			else
			{
				closedir(dir);
			}
		}

		loggerfd = open(logPath, O_WRONLY | O_APPEND | O_CREAT, 0644);

		if (-1 == loggerfd)
		{
			fprintf(stdout,"can not open log file %s ,the program will exit\n",logPath);
			exit(0);
		}

		fprintf(stdout,"open log file %s successfully\n", logPath);
	}
}

void fl_logger_close()
{
	close(loggerfd);
}

/*
 * level 0 : info
 * level 1 : warning
 * level 2 : error
 * level 8888: debug
 * level 9999 : exit info
 * */
void fl_log(int level, const char * msg, ...)
{
	if (-1 == loggerfd) return;
	char levelStr[20];
	switch(level)
	{
	case 0:
		strcpy(levelStr, "[INFO]");
		break;
	case 1:
		strcpy(levelStr, "[WARNING]");
		break;
	case 2:
		strcpy(levelStr, "[ERROR]");
		break;
	case 8888:
		strcpy(levelStr, "[DEBUG]");
		break;
	case 9999:
		strcpy(levelStr, "[EXIT]");
		break;
	default:
		strcpy(levelStr, "[INFO]");
		break;
	}


	char vbuf[2048];
	va_list vlist;
	va_start(vlist, msg);
	vsnprintf(vbuf, sizeof(vbuf), msg, vlist);
	va_end(vlist);


	time_t ct;
	struct tm * ctm;
	ct = time(NULL);
	ctm = localtime(&ct);

	char logBuf[2048];
	snprintf(logBuf, sizeof(logBuf), "%s %02d:%02d:%02d : %s",levelStr,ctm->tm_hour,ctm->tm_min,ctm->tm_sec,vbuf);
	int len = strlen(logBuf);
	if ('\n' != logBuf[len-1])
	{
		logBuf[len++] = '\n';
		logBuf[len++] = 0;
	}
	write(loggerfd,logBuf,strlen(logBuf));

}

void fl_debug_log(const char * msg, ...)
{
	char vbuf[2048];
	va_list vlist;
	va_start(vlist, msg);
	vsnprintf(vbuf, sizeof(vbuf), msg, vlist);
	va_end(vlist);
	int len = strlen(vbuf);
	if ('\n' != vbuf[len-1])
	{
		vbuf[len++] = '\n';
		vbuf[len++] = 0;
	}
	fl_log(8888,vbuf);
}






