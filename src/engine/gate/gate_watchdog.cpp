#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "net.h"
#include "gate.h"
#include "logger.h"
#include "buffer.h"
#include "ByteArray.h"
#include "BitRecord.h"
#include "MsgQueue.h"
#include "envirment.h"

static void * s_watchdog_thread(void * arg);
static void s_watchdog_handle_message(struct fl_message_data * message);
static int s_watchdog_listen_fd;
static class fl_connection * s_backgate_connections;
static int s_run_state = 0;
static int MAX_MESSAGE_LENGTH = 2097152;
static bool s_is_net_gate_watchdog = false;

bool fl_start_net_gate_watchdog_server()
{
	struct sockaddr_in server_addr;
	s_watchdog_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == s_watchdog_listen_fd)
	{
		fl_log(2, "[Gate_Watchdog]: gate watchdog server socket failed, errno:%d\n", errno);
		return false;
	}

	//set server information
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((uint16_t)fl_getenv("gate_server_ctrl_port", 6701));
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //any ip is ok

	//bind server
	if (-1 == (bind(s_watchdog_listen_fd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))))
	{
		close(s_watchdog_listen_fd);
		fl_log(2, "[Gate_Watchdog]: gate watchdog server bind error, errno:%d\n", errno);
		return false;
	}

	//listen server
	if (-1 == listen(s_watchdog_listen_fd, 1024))
	{
		close(s_watchdog_listen_fd);
		fl_log(2, "[Gate_Watchdog]: gate watchdog server listen error, errno:%d\n", errno);
		return false;
	}

	char * addr_tmp;
	uint16_t port_tmp;
	addr_tmp = inet_ntoa(server_addr.sin_addr);
	port_tmp = ntohs(server_addr.sin_port);

	fl_log(0,"[Gate_Watchdog]: gate watchdog server listen on %s : %d\n", addr_tmp, port_tmp);

	MAX_MESSAGE_LENGTH = fl_getenv("message_max_length", 131072);

	s_backgate_connections = new fl_connection[32]();
	for (int i=0;i<32;++i)
	{
		s_backgate_connections[i].SetIndex(i);
		s_backgate_connections[i].SetMaxMessageLength(MAX_MESSAGE_LENGTH);
		s_backgate_connections[i].SetRecvCallBack(s_watchdog_handle_message);
	}

	s_run_state = 1;

	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, s_watchdog_thread, NULL);

	s_is_net_gate_watchdog = true;

	return true;
}

void fl_stop_net_gate_watchdog_server()
{
	close(s_watchdog_listen_fd);
	s_run_state = 0;
	for (int i=0;i<32;++i)
	{
		s_backgate_connections[i].Close();
	}
	delete [] s_backgate_connections;
}

static void * s_watchdog_thread(void * arg)
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(struct sockaddr);
	fd_set readset;
	int maxfd, selectn, clientfd, i;
	struct timeval tv;
	uint32_t session = 0;
	while (true)
	{
		if (s_run_state == 0) break;
		maxfd = -1;
		FD_ZERO(&readset);
		FD_SET(s_watchdog_listen_fd, &readset);
		maxfd = s_watchdog_listen_fd > maxfd ? s_watchdog_listen_fd : maxfd;

		tv.tv_sec = 0;
		tv.tv_usec = 1000000;  //1S
		selectn = select(maxfd + 1, &readset, NULL, NULL, &tv);

		if (selectn <= 0)
		{
			if (selectn < 0)
			{
				fl_log(2,"[Gate_Watchdog]: select error, errno:%d\n", errno);
			}
			continue;
		}

		clientfd = -1;

		if (FD_ISSET(s_watchdog_listen_fd, &readset))
		{
			//new client
			clientfd = accept(s_watchdog_listen_fd,(struct sockaddr*)&client_addr,&client_len);
			if (-1 == clientfd)
			{
				fl_log(2,"[Gate_Watchdog]: Accept client error,errno = %d\n",errno);
			}
			else
			{
				for (i=0;i<32;++i)
				{
					if (-1 == s_backgate_connections[i].GetSockfd()) continue;
					s_backgate_connections[i].SetSocketInfo(clientfd, ++session);
					s_backgate_connections[i].SetAddrInfo(&client_addr);
					if (session > 0XFFFFFFFE)
					{
						session = 0;
					}
					break;
				}
			}
			--selectn;
		}

		if (selectn > 0)
		{
			for (i=0;i<32;++i)
			{
				if (-1 == s_backgate_connections[i].GetSockfd()) continue;
				if (clientfd == s_backgate_connections[i].GetSockfd()) continue;
				if (FD_ISSET(s_backgate_connections[i].GetSockfd(), &readset))
				{
					if (false == s_backgate_connections[i].Recv())
					{
						s_backgate_connections[i].Close();
					}
					--selectn;
				}
				if (0 == selectn) break;
			}
		}

	}
	pthread_exit(0);
}

static std::vector<std::string> msg;
static void s_watchdog_handle_message(struct fl_message_data * message)
{
	ReadByteArray readByteArray;
	readByteArray.SetReadContent(message->data, message->length);

	char str[1024];
	int j = 0;
	int clientfd = message->fd;
	std::string tmp = readByteArray.ReadString();
	fl_free_message_data(message);
	msg.clear();
	const char * ch = tmp.c_str();
	while (*ch != '\0')
	{
		if (*ch == ',')
		{
			str[j] = '\0';
			msg.push_back(std::string(str));
			j = 0;
		}
		else
		{
			str[j++] = *ch;
		}
		++ch;
	}

	if (msg[0] == "client")
	{
		if (msg.size() != 3)
		{
			return;
		}
	}
	else if (msg[0] == "shutdown")
	{
		fl_log(1,"[Gate_Watchdog]: receive a shutdown command from client %d,the Gate Server will shutdown\n", clientfd);
		fl_stop_net_gate_server();
		fl_stop_net_gate_watchdog_server();
	}
	else
	{
		fl_log(2,"[Gate_Watchdog]: Unknow command %s\n", msg[0].c_str());
	}
}
