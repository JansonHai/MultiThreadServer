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

static int s_listen_fd = -1;
static uint32_t s_session = 0;
static int s_login_server_pid = -1;

void s_login_server_loop();

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
		_exit(0);
	}

	//set server information
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((uint16_t)fl_getenv("login_server_port",6666));
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //any ip is ok

	//set socket reuse address
	bool reuseAddr = true;
	setsockopt (s_listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(bool));

	//bind server
	if (-1 == (bind(s_listen_fd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))))
	{
		fl_log(2, "login server bind error, errno:%d\n", errno);
		_exit(0);
	}

	//listen server
	if (-1 == listen(s_listen_fd, 1024))
	{
		fl_log(2, "login server listen error, errno:%d\n", errno);
		_exit(0);
	}

	char * addr_tmp;
	uint16_t port_tmp;
	addr_tmp = inet_ntoa(server_addr.sin_addr);
	port_tmp = ntohs(server_addr.sin_port);

	fl_log(0,"login server listen on %s : %d\n", addr_tmp, port_tmp);

	s_login_server_loop();
	_exit(0);
	return true;
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

	while (true)
	{
		maxfd = -1;
		FD_ZERO(&readset);
		FD_SET(s_listen_fd, &readset);
		maxfd = s_listen_fd > maxfd ? s_listen_fd : maxfd;

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
					if (-1 == g_client_conn_fds[i])
					{
//						fl_log(1,"Accept client %d,client index = %d\n", clientfd, i);
						g_client_conn_sessions[i] = ++s_session;
						g_client_conn_fds[i]= clientfd;
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
