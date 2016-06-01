#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "login.h"
#include "logger.h"
#include "buffer.h"
#include "ByteArray.h"

static int s_listen_fd = -1;
static uint32_t s_session = 0;
static int s_login_server_pid = -1;
static int s_run_state = 0;
static int MAX_CLIENT_CONNECTIONS = 2048;

static struct login_connection * connections;

void s_login_server_loop();

void fl_stop_login_server()
{
	s_run_state = 0;
	usleep(1500000);  //1.5S
	free(connections);
	_exit(0);
}

bool fl_start_login_server()
{
	s_login_server_pid = fork();

	if (s_login_server_pid < 0)
	{
		fl_log(2,"login server fork failed,errno: %d\n",errno);
		return false;
	}

	if (s_login_server_pid > 0)
	{
		fl_log(2,"login server fork successfully,pid = %d\n", s_login_server_pid);
		return true;
	}

	struct sockaddr_in server_addr;
	s_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == s_listen_fd)
	{
		fl_log(2, "login server socket failed, errno:%d\n", errno);
		fl_stop_login_server();
	}

	MAX_CLIENT_CONNECTIONS = fl_getenv("max_client_connection",2048);
	connections = (struct login_connection * )malloc(MAX_CLIENT_CONNECTIONS * sizeof(struct login_connection));
	for (int i=0;i<MAX_CLIENT_CONNECTIONS;++i)
	{
		connections[i].fd = -1;
		connections[i].session = 0;
	}

	//set server information
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((uint16_t)fl_getenv("login_server_port",6666));
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //any ip is ok

	//set socket reuse address
//	bool reuseAddr = true;
//	setsockopt (s_listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(bool));

	//bind server
	if (-1 == (bind(s_listen_fd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))))
	{
		fl_log(2, "login server bind error, errno:%d\n", errno);
		fl_stop_login_server();
	}

	//listen server
	if (-1 == listen(s_listen_fd, 1024))
	{
		fl_log(2, "login server listen error, errno:%d\n", errno);
		fl_stop_login_server();
	}

	char * addr_tmp;
	uint16_t port_tmp;
	addr_tmp = inet_ntoa(server_addr.sin_addr);
	port_tmp = ntohs(server_addr.sin_port);

	fl_log(0,"login server listen on %s : %d\n", addr_tmp, port_tmp);

	s_run_state = 1;
	fl_init_buffer();
	s_login_server_loop();
	fl_stop_login_server();
	return true;
}

static void * s_read_thread(void * arg)
{
	while (s_run_state != 0)
	{
		usleep(10000);  //10ms;
	}
	pthread_exit(0);
}

static void * s_write_thread(void * arg)
{
	while (s_run_state != 0)
	{
		usleep(10000);  //10ms;
	}
	pthread_exit(0);
}

void s_login_server_loop()
{
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

	//start main loop
	fd_set readset;
	int maxfd = -1, selectn = 0;
	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	//create read thread
	pthread_create(&tid, &attr, s_read_thread, NULL);

	//create write thread
	pthread_create(&tid, &attr, s_write_thread, NULL);

	while (0 != s_run_state)
	{
		maxfd = -1;
		FD_ZERO(&readset);
		FD_SET(s_listen_fd, &readset);
		maxfd = s_listen_fd > maxfd ? s_listen_fd : maxfd;

		fl_do_real_client_close();

		for (i=0;i < MAX_CLIENT_CONNECTIONS;++i)
		{
			if (connections[i].fd > maxfd)
			{
				maxfd = connections[i].fd;
				FD_SET(connections[i].fd, &readset);
			}
		}


		tv.tv_sec = 0;
		tv.tv_usec = 1000000;  //1S
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

		if (FD_ISSET(s_listen_fd,&readset))
		{
			//new client
			clientfd = accept(s_listen_fd,(struct sockaddr*)&client_addr,&client_len);
			if (-1 == clientfd)
			{
				fl_log(2,"Accept client error,errno = %d\n",errno);
			}
			else
			{
				for (i=0;i<MAX_CLIENT_CONNECTIONS;++i)
				{
					if (-1 == connections[i].fd)
					{
//						fl_log(1,"Accept client %d,client index = %d\n", clientfd, i);
						connections[i].session = ++s_session;
						connections[i].fd= clientfd;
						connections[i].addr = client_addr;
						if (s_session > 0XFFFFFFFE)
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
				if (-1 == connections[i].fd) continue;
				if (clientfd == connections[i].fd) continue;
				if (FD_ISSET(connections[i].fd, &readset))
				{
					--selectn;
				}
				if (0 == selectn) break;
			}
		}

	}   //end of while(true)
}  //server main loop end
