#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "net.h"
#include "exit.h"
#include "logger.h"
#include "global.h"

static void s_exit_handle(void * args)
{

}

void fl_net_init()
{
	fl_add_main_exit_handle(s_exit_handle);

}

bool fl_send_message(int fd,const char * data,int length)
{
	uint32_t netlength = htonl(length);
	int sendn = 0;
	sendn = send(fd, &netlength, 4, 0);
	if (sendn < 0)
	{
		fl_log(2,"send data lengtn error,errno: %d\n", errno);
		return false;
	}
	while (length > 0)
	{
		sendn = send(fd, data, length, 0);
		if (sendn < 0)
		{
			fl_log(2,"send data error,errno: %d\n", errno);
			return false;
		}
		length -= sendn;
		data += sendn;
	}
	return true;
}

static pthread_mutex_t s_malloc_message_lock = PTHREAD_MUTEX_INITIALIZER;
void fl_malloc_message_data(struct fl_message_data * &message, int index, int fd, int session)
{
	pthread_mutex_lock(&s_malloc_message_lock);
	message = (struct fl_message_data *)malloc(sizeof(struct fl_message_data));
	message->index = index;
	message->fd = fd;
	message->session = session;
	message->header = 0;
	message->headerReadLength = 0;
	message->data = NULL;
	message->length = 0;
	message->readLength = 0;
	message->isReadFinish = false;
	message->isDropMessage = false;
	pthread_mutex_unlock(&s_malloc_message_lock);
}

void fl_reset_message_data(struct fl_message_data * &message, int index, int fd, int session)
{
	if (NULL == message) return;
	if (NULL != message->data)
	{
		free(message->data);
	}
	message->index = index;
	message->fd = fd;
	message->session = session;
	message->header = 0;
	message->headerReadLength = 0;
	message->data = NULL;
	message->length = 0;
	message->readLength = 0;
	message->isReadFinish = false;
	message->isDropMessage = false;
}

void fl_drop_message_data(struct fl_message_data * &message)
{
	if (NULL == message) return;
	if (NULL != message->data)
	{
		free(message->data);
	}
	message->index = -1;
	message->fd = -1;
	message->session = 0;
	message->header = 0;
	message->headerReadLength = 0;
	message->data = NULL;
	message->length = 0;
	message->readLength = 0;
	message->isReadFinish = false;
	message->isDropMessage = false;
}

void fl_free_message_data(struct fl_message_data * &message)
{
	if (NULL == message) return;
	if (NULL != message->data)
	{
		free(message->data);
	}
	free(message);
}



