#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include "login.h"
#include "logger.h"

static int s_listen_fd = -1;
static uint32_t s_session = 0;

void fl_start_login_server()
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

	//create read thread
	int readThreadNum = fl_getenv("read_thread_num",16);
	fprintf(stdout,"createing read thread,thread num:%d\n", readThreadNum);
	fl_log(0, "createing read thread,thread num:%d\n", readThreadNum);

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
