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

static int listen_fd = -1;
static int s_session = 0;
extern bool g_exit_flag;

extern int MAX_CLIENT_CONNECTIONS;
extern uint32_t MAX_MESSAGE_LENGTH;
extern int * g_client_conn_fds;
extern int * g_client_conn_sessions;
extern struct fl_net_message_queue * g_client_conn_mq;
extern pthread_mutex_t * g_client_conn_mutex;

static void s_clear_server(void * arg)
{

	for (int i=0;i<MAX_CLIENT_CONNECTIONS;++i)
	{
		if (-1 != g_client_conn_fds[i])
		{
			close(g_client_conn_fds[i]);
			g_client_conn_fds[i] = -1;
		}
		pthread_mutex_destroy(&g_client_conn_mutex[i]);
	}
	if (-1 != listen_fd)
	{
		fl_exit_log("close listen fd\n");
		close(listen_fd);
	}
	listen_fd = -1;
	free(g_client_conn_fds);
	free(g_client_conn_sessions);
	free(g_client_conn_mq);
	mysql_library_end();
}

void fl_server_start()
{
	fl_add_main_exit_handle(s_clear_server);

	MAX_CLIENT_CONNECTIONS = fl_getenv("max_client_connection",2048);
	g_client_conn_fds = (int *)malloc(MAX_CLIENT_CONNECTIONS * sizeof(int));
	g_client_conn_mq = (struct fl_net_message_queue *)malloc(MAX_CLIENT_CONNECTIONS * sizeof(struct fl_net_message_queue));
	g_client_conn_mutex = (pthread_mutex_t *)malloc(MAX_CLIENT_CONNECTIONS * sizeof(pthread_mutex_t));
	g_client_conn_sessions = (int *)malloc(MAX_CLIENT_CONNECTIONS * sizeof(int));
	for (int i=0;i<MAX_CLIENT_CONNECTIONS;++i)
	{
		g_client_conn_fds[i] = -1;
		pthread_mutex_init(&g_client_conn_mutex[i], NULL);
	}

	int listenfd = -1;
	struct sockaddr_in server_addr;
	listenfd = socket(AF_INET,SOCK_STREAM,0);
	if (-1 == listenfd)
	{
		fprintf(stderr, "socket failed,errno:%d\n", errno);
		fl_log(2, "socket failed,errno:%d\n", errno);
		exit(0);
	}

	//set server information
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((uint16_t)fl_getenv("server_port",6666));
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //any ip is ok

	bool reuseAddr = true;
	setsockopt (listenfd ,SOL_SOCKET ,SO_REUSEADDR, &reuseAddr,sizeof(bool));
	//bind server
	if (-1 == (bind(listenfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))))
	{
		fprintf(stderr, "bind error,errno:%d\n", errno);
		fl_log(2, "bind error,errno:%d\n", errno);
		exit(0);
	}

	listen_fd = listenfd;

	//listen server
	if (-1 == listen(listenfd,1024))
	{
		fprintf(stderr,"listen error,errno:%d\n", errno);
		fl_log(2, "listen error,errno:%d\n", errno);
		exit(0);
	}

	char * addr_tmp;
	uint16_t port_tmp;
	addr_tmp = inet_ntoa(server_addr.sin_addr);
	port_tmp = ntohs(server_addr.sin_port);

	fprintf(stdout,"listen on %s : %d\n", addr_tmp, port_tmp);
	fl_log(0,"listen on %s : %d\n", addr_tmp, port_tmp);

}

static void * s_work_thread_run(void * arg)
{
	struct fl_message_data * message = NULL;
	char recvBuf[4096];
	char sendBuf[4096];
	while (true)
	{
		if (true == g_exit_flag) break;
		fl_pop_handle_message(message);
		if (NULL == message)
		{
			usleep(1000);  //sleep 1ms;
			continue;
		}
		strncpy(recvBuf, message->data, message->length);
		sprintf(sendBuf,"I receive your message %s",recvBuf);
		fl_send_message(message->fd, sendBuf,strlen(sendBuf) + 1);
//		fl_close_client_fd(message->fd);
		fl_free_message_data(message);
	}
	pthread_exit(0);
}


static void * s_read_thread_run(void * arg)
{
	int index;
	int client_fd;
	int client_session;
	struct fl_message_data * message;
	int readn;
	uint32_t readLeft;
	uint32_t netLength = 0;
	uint32_t length = 0;

	while (true)
	{
	__read_thread_begin:
		if (true == g_exit_flag) break;
		index = fl_pop_read_fd_index();

		if (-1 == index)
		{
			usleep(1000);  //sleep 1ms;
			continue;
		}

		pthread_mutex_lock(&g_client_conn_mutex[index]);

		client_fd = g_client_conn_fds[index];
		if (-1 == client_fd)
		{
			fl_reset_read_fd_index(index);
			pthread_mutex_unlock(&g_client_conn_mutex[index]);
			continue;
		}
		client_session = g_client_conn_sessions[index];

		message = g_client_conn_mq[index].current_data;
		if (NULL == message)
		{
			fl_malloc_message_data(message, client_fd, client_session);
			g_client_conn_mq[index].current_data = message;
		}
		else
		{
			if (client_session != message->session)
			{
				fl_reset_message_data(message, client_fd, client_session);
				g_client_conn_mq[index].current_data = message;
			}
		}
		if (NULL == message->data)
		{
			readLeft = 4;
			while (readLeft > 0)
			{
				readn = recv(client_fd, (&netLength) + 4 - readLeft, readLeft, MSG_DONTWAIT);
				if (readn <= 0)
				{
					if (EAGAIN == errno)
					{
						continue;
					}
					else
					{
						fl_log(2,"read length error,errno: %d\n",errno);
						fl_close_client_index(index);
						fl_reset_read_fd_index(index);
						pthread_mutex_unlock(&g_client_conn_mutex[index]);
						goto __read_thread_begin;
					}
				}
				readLeft -= readn;
			}
			length = ntohl(netLength);
			if (length <= MAX_MESSAGE_LENGTH)
			{
				message->length = length;
				message->data = (char *)malloc(length * sizeof(char));
				message->readLength = 0;
				message->isReadFinish = false;
			}
			else
			{
				fl_log(2,"client %d send message too large,length : %d,this message will drop\n", client_fd, length);
				fl_drop_message_data(message);
				pthread_mutex_unlock(&g_client_conn_mutex[index]);
				continue;
			}
		}
		while (true)
		{
			readLeft = message->length - message->readLength;
			if (readLeft > 4096)
			{
				readLeft = 4096;
			}
			readn = recv(message->fd, message->data + message->readLength, readLeft, MSG_DONTWAIT);
			if (readn <= 0)
			{
				if (EAGAIN == errno)
				{
					continue;
				}
				else
				{
					fl_log(2,"read data error,errno: %d\n", errno);
					fl_close_client_index(index);
					break;
				}
			}
			message->readLength += readn;
			if (message->readLength == message->length)
			{
				message->isReadFinish = true;
				fl_push_handle_message(message);
				g_client_conn_mq[index].current_data = NULL;
				break;
			}
		}
		fl_reset_read_fd_index(index);
		pthread_mutex_unlock(&g_client_conn_mutex[index]);
	}
	pthread_exit(0);
}

void fl_server_main_loop()
{
	//init the database
	fl_database_init();

	//init net message handle
	fl_net_init();

	int i;
	int clientfd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(struct sockaddr);

	fprintf(stdout, "starting main loop\n");
	fl_log(0, "starting main loop\n");

	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	MAX_MESSAGE_LENGTH = fl_getenv("message_max_length",2097152);

	//create read thread
	int readThreadNum = fl_getenv("read_thread_num",16);
	fprintf(stdout,"createing read thread,thread num:%d\n", readThreadNum);
	fl_log(0, "createing read thread,thread num:%d\n", readThreadNum);
	for (int i=0; i<readThreadNum; ++i)
	{
		pthread_create(&tid, &attr, s_read_thread_run, NULL);
	}

	//create work thread
	int workThreadNum = fl_getenv("work_thread_num",256);
	fprintf(stdout,"createing work thread,thread num:%d\n", workThreadNum);
	fl_log(0, "createing work thread,thread num:%d\n", workThreadNum);
	for (int i=0; i<workThreadNum; ++i)
	{
		pthread_create(&tid, &attr, s_work_thread_run, NULL);
	}

	//start main loop
	fd_set readset;
	int maxfd = -1, selectn = 0;
	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	while (true)
	{
		maxfd = -1;
		FD_ZERO(&readset);
		FD_SET(listen_fd, &readset);
		maxfd = listen_fd > maxfd ? listen_fd : maxfd;

		fl_do_real_client_close();

		for (i=0;i < MAX_CLIENT_CONNECTIONS;++i)
		{
			if (g_client_conn_fds[i] > maxfd)
			{
				maxfd = g_client_conn_fds[i];
				FD_SET(g_client_conn_fds[i], &readset);
			}
		}


		tv.tv_sec = 10;
		tv.tv_usec = 0;
		selectn = select(maxfd + 1, &readset, NULL, NULL, &tv);

		if (selectn <= 0)
		{
			if (selectn < 0)
			{
				fl_log(2,"select error, errno:%d\n", errno);
			}
			continue;
		}

//		fprintf(stdout,"select %d fd\n",selectn);
		clientfd = -1;

		if (FD_ISSET(listen_fd,&readset))
		{
			//new client
			clientfd = accept(listen_fd,(struct sockaddr*)&client_addr,&client_len);
			if (-1 == clientfd)
			{
				fl_log(2,"Accept client error,errno = %d\n",errno);
			}
			else
			{
				for (i=0;i<MAX_CLIENT_CONNECTIONS;++i)
				{
					if (-1 == g_client_conn_fds[i])
					{
//						fl_log(1,"Accept client %d,client index = %d\n", clientfd, i);
						g_client_conn_sessions[i] = ++s_session;
						g_client_conn_fds[i]= clientfd;
						if (s_session > 0x7fffff0)
						{
							s_session = 0;
						}
						break;
					}
				}
			}
			--selectn;
		}

		if (selectn > 0)
		{
			for (i=0;i<MAX_CLIENT_CONNECTIONS;++i)
			{
				if (-1 == g_client_conn_fds[i]) continue;
				if (clientfd == g_client_conn_fds[i]) continue;
				if (FD_ISSET(g_client_conn_fds[i], &readset))
				{
					--selectn;
					fl_push_read_fd_index(i);
				}
				if (0 == selectn) break;
			}
		}

	}   //end of while(true)
}  //server main loop end




