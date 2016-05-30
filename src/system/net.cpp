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

extern int MAX_CLIENT_CONNECTIONS;
extern int * g_client_conn_fds;
extern int * g_client_conn_sessions;

static bool * s_client_close_flag;
static bool * s_push_index_flag;

static void s_exit_handle(void * args)
{
	free(s_client_close_flag);
}

void fl_net_init()
{
	fl_add_main_exit_handle(s_exit_handle);
	s_client_close_flag = (bool *)malloc(MAX_CLIENT_CONNECTIONS * sizeof(bool));
	s_push_index_flag = (bool *)malloc(MAX_CLIENT_CONNECTIONS * sizeof(bool));
	for (int i=0;i<MAX_CLIENT_CONNECTIONS;++i)
	{
		s_client_close_flag[i] = false;
		s_push_index_flag[i] = false;
	}
}

bool fl_send_message(int fd,const char * data,uint32_t length)
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
void fl_malloc_message_data(struct fl_message_data * &message, int fd, int session)
{
	pthread_mutex_lock(&s_malloc_message_lock);
	message = (struct fl_message_data *)malloc(sizeof(struct fl_message_data));
	message->fd = fd;
	message->session = session;
	message->data = NULL;
	message->length = 0;
	message->readLength = 0;
	message->isReadFinish = false;
	pthread_mutex_unlock(&s_malloc_message_lock);
}

void fl_reset_message_data(struct fl_message_data * &message, int fd, int session)
{
	if (NULL != message->data)
	{
		free(message->data);
	}
	message->fd = fd;
	message->session = session;
	message->data = NULL;
	message->length = 0;
	message->readLength = 0;
	message->isReadFinish = false;
}

void fl_drop_message_data(struct fl_message_data * &message)
{
	if (NULL != message->data)
	{
		free(message->data);
	}
	message->fd = -1;
	message->session = -1;
	message->data = NULL;
	message->length = 0;
	message->readLength = 0;
	message->isReadFinish = false;
}

void fl_free_message_data(struct fl_message_data * &message)
{
	if (NULL != message->data)
	{
		free(message->data);
	}
	free(message);
}

static pthread_mutex_t s_get_read_fd_lock = PTHREAD_MUTEX_INITIALIZER;
static const int FD_INDEX_QUEUE_LENGTH = 512000;
static int s_read_fd_index_queue[FD_INDEX_QUEUE_LENGTH];
static int s_fd_queue_head = 0;
static int s_fd_queue_tail = 0;
static int s_fd_queue_length = 0;

void fl_push_read_fd_index(int index)
{
	pthread_mutex_lock(&s_get_read_fd_lock);
	if (false == s_client_close_flag[index] && false == s_push_index_flag[index])
	{
		s_push_index_flag[index] = true;
		s_read_fd_index_queue[s_fd_queue_tail++] = index;
		if (s_fd_queue_tail >= FD_INDEX_QUEUE_LENGTH) s_fd_queue_tail = 0;
		++s_fd_queue_length;
	}
	pthread_mutex_unlock(&s_get_read_fd_lock);
}

int fl_pop_read_fd_index()
{
	int index = -1;
	pthread_mutex_lock(&s_get_read_fd_lock);
	if (s_fd_queue_length > 0)
	{
		index = s_read_fd_index_queue[s_fd_queue_head++];
		if (s_fd_queue_head >= FD_INDEX_QUEUE_LENGTH) s_fd_queue_head = 0;
		--s_fd_queue_length;
	}
	pthread_mutex_unlock(&s_get_read_fd_lock);
	return index;
}

void fl_reset_read_fd_index(int index)
{
	pthread_mutex_lock(&s_get_read_fd_lock);
	s_push_index_flag[index] = false;
	pthread_mutex_unlock(&s_get_read_fd_lock);
}

static pthread_mutex_t s_get_handle_message_lock = PTHREAD_MUTEX_INITIALIZER;
static const int HANDLE_MESSAGE_QUEUE_LENGTH = 1024000;
static struct fl_message_data * s_handle_message_queue[HANDLE_MESSAGE_QUEUE_LENGTH];
static int s_message_queue_head = 0;
static int s_message_queue_tail = 0;
static int s_message_queue_length = 0;

void fl_push_handle_message(struct fl_message_data * message)
{
	pthread_mutex_lock(&s_get_handle_message_lock);
	s_handle_message_queue[s_message_queue_tail++] = message;
	if (s_message_queue_tail >= HANDLE_MESSAGE_QUEUE_LENGTH) s_message_queue_tail = 0;
	++s_message_queue_length;
	pthread_mutex_unlock(&s_get_handle_message_lock);
}


void fl_pop_handle_message(struct fl_message_data * &message)
{
	pthread_mutex_lock(&s_get_handle_message_lock);
	message = NULL;
	if (s_message_queue_length > 0)
	{
		message = s_handle_message_queue[s_message_queue_head++];
		if (s_message_queue_head >= HANDLE_MESSAGE_QUEUE_LENGTH) s_message_queue_head = 0;
		--s_message_queue_length;
	}
	pthread_mutex_unlock(&s_get_handle_message_lock);
}

static pthread_mutex_t s_close_socket_lock = PTHREAD_MUTEX_INITIALIZER;

void fl_close_client_fd(int fd)
{
	pthread_mutex_lock(&s_close_socket_lock);
	for (int i=0;i<MAX_CLIENT_CONNECTIONS;++i)
	{
		if (g_client_conn_fds[i] == fd)
		{
			s_client_close_flag[i] = true;
			break;
		}
	}
	pthread_mutex_unlock(&s_close_socket_lock);
}

void fl_close_client_index(int index)
{
	pthread_mutex_lock(&s_close_socket_lock);
	if (index < MAX_CLIENT_CONNECTIONS)
	{
		if (-1 != g_client_conn_fds[index])
		{
			s_client_close_flag[index] = true;
		}
	}
	pthread_mutex_unlock(&s_close_socket_lock);
}

static struct linger s_linger;

void fl_do_real_client_close()
{
	pthread_mutex_lock(&s_close_socket_lock);
	for (int i=0;i<MAX_CLIENT_CONNECTIONS;++i)
	{
		if (true == s_client_close_flag[i])
		{
			if (-1 != g_client_conn_fds[i])
			{
				s_linger.l_onoff = 1;  //set linger to make data send
				s_linger.l_linger = 3;  //set linger time = 3s
				setsockopt(g_client_conn_fds[i], SOL_SOCKET, SO_LINGER, &s_linger, sizeof(struct linger));
				close(g_client_conn_fds[i]);
				g_client_conn_fds[i] = -1;
			}
			g_client_conn_sessions[i] = -1;
			s_client_close_flag[i] = false;
		}
	}
	pthread_mutex_unlock(&s_close_socket_lock);
}



