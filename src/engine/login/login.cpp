#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include "login.h"
#include "logger.h"
#include "buffer.h"
#include "ByteArray.h"
#include "BitRecord.h"
#include "MsgQueue.h"
#include "envirment.h"

static int s_listen_fd = -1;
static int s_login_server_pid = -1;
static int s_run_state = 0;
static int MAX_CLIENT_CONNECTIONS = 2048;
static int MAX_MESSAGE_LENGTH = 2097152;

static class BitRecord s_conn_bit_record;
static class BitRecord s_read_bit_record;
static class MsgQueue<int> s_read_msg_queue;
static class MsgQueue<struct fl_message_data *> s_work_msg_queue;
static class fl_connection * s_connections;
static pthread_mutex_t s_close_mutex;

static int s_net_gate_server_fd = -1;

static void s_login_server_loop();
static void s_recv_data_callback(struct fl_message_data * message);
static bool s_connect_to_gate_server();

void fl_stop_login_server()
{
	if (-1 != s_listen_fd)
	{
		close(s_listen_fd);
	}
	if (-1 != s_net_gate_server_fd)
	{
		close(s_net_gate_server_fd);
	}
	s_run_state = 0;
	fl_stop_login_watchdog_server();
	usleep(1000000 * 3);  //3S
	for (int i=0; i<MAX_CLIENT_CONNECTIONS; ++i)
	{
		s_connections[i].Close();
	}
	delete [] s_connections;
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
		fl_log(0,"login server fork successfully,pid = %d\n", s_login_server_pid);
		return true;
	}

	signal(SIGINT, SIG_IGN);

	struct sockaddr_in server_addr;
	s_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == s_listen_fd)
	{
		fl_log(2, "login server socket failed, errno:%d\n", errno);
		fl_stop_login_server();
	}

	MAX_MESSAGE_LENGTH = fl_getenv("message_max_length", 131072);
	MAX_CLIENT_CONNECTIONS = fl_getenv("max_client_connection", 2048);
	s_connections = new fl_connection[MAX_CLIENT_CONNECTIONS]();
	for (int i=0;i<MAX_CLIENT_CONNECTIONS;++i)
	{
		s_connections[i].SetIndex(i);
		s_connections[i].SetMaxMessageLength(MAX_MESSAGE_LENGTH);
		s_connections[i].SetRecvCallBack(s_recv_data_callback);
	}

	//init bit flag to mark connection
	s_conn_bit_record.SetBitsLength(MAX_CLIENT_CONNECTIONS);
	s_read_bit_record.SetBitsLength(MAX_CLIENT_CONNECTIONS);
	pthread_mutex_init(&s_close_mutex, NULL);

	//set server information
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((uint16_t)fl_getenv("login_server_port",6600));
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //any ip is ok

	//set socket reuse address
//	bool reuseAddr = true;
//	setsockopt (s_listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(bool));

	//bind server
	if (-1 == (bind(s_listen_fd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))))
	{
		close(s_listen_fd);
		fl_log(2, "login server bind error, errno:%d\n", errno);
		fl_stop_login_server();
	}

	//listen server
	if (-1 == listen(s_listen_fd, 1024))
	{
		close(s_listen_fd);
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
	if (false == fl_start_login_watchdog_server())
	{
		fl_log(2,"strt login watchdog server failed..\n");
		fl_stop_login_server();
		return false;
	}

	if (false == s_connect_to_gate_server())
	{
		fl_log(2,"strt login watchdog server failed..\n");
		fl_stop_login_server();
		return false;
	}

	s_login_server_loop();
	fl_stop_login_server();
	return true;
}

static bool s_connect_to_gate_server()
{
	s_net_gate_server_fd = socket(AF_INET,SOCK_STREAM,0);
	if (-1 == s_net_gate_server_fd)
	{
		client_log(2,"login server create connect socket failed, errno: %d\n", errno);
		return false;
	}

	struct sockaddr_in server_addr;

	const char * ip = fl_getenv("gate_server_ip", "127.0.0.1");
	int port = fl_getenv("gate_server_ctrl_port", 6701);

	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);

	if (-1 == connect(s_net_gate_server_fd,(struct sockaddr*)&server_addr, sizeof(struct sockaddr)))
	{
		client_log(2,"login server connect to gate server [%s:%d] failed, errno:%d\n", ip, port, errno);
		return false;
	}
	return true;
}

void fl_login_send_message_to_client(int index, uint32_t session, const char * data, int length)
{
	class fl_connection * conn = &s_connections[index];
	if (conn->GetSession() == session)
	{
		if (false == conn->Send(data, length))
		{
			conn->Close();
			s_conn_bit_record.ResetBit(index);
		}
	}
}

static void s_recv_data_callback(struct fl_message_data * message)
{
	s_work_msg_queue.push_message(message);
}

static void * s_read_thread(void * arg)
{
	int index = -1;
	bool result;
	class fl_connection * conn = NULL;
	while (true)
	{
		if (0 == s_run_state) break;
		result = s_read_msg_queue.pop_message(index);
		if (false == result || -1 == index)
		{
			usleep(10000);  //10ms;
			continue;
		}
		conn = &s_connections[index];
		if (false == conn->Recv())
		{
			conn->Close();
			s_conn_bit_record.ResetBit(index);
		}
		s_read_bit_record.ResetBit(index);
		conn = NULL;
	}
	pthread_exit(0);
}

static void * s_work_thread(void * arg)
{
//	int index, readLeft, readn, length;
	bool result;
	struct fl_message_data * message;
	class fl_connection * conn;
	ReadByteArray * readByteArray;
	while (true)
	{
		if (0 == s_run_state) break;
		message = NULL;
		result = s_work_msg_queue.pop_message(message);
		if (false == result || NULL == message)
		{
			usleep(10000);  //10ms;
			continue;
		}

		conn = &s_connections[message->index];
		if (conn->GetSession() != message->session)
		{
			fl_free_message_data(message);
			message = NULL;
			continue;
		}

		//handle
		readByteArray = new ReadByteArray();
		readByteArray->SetReadContent(message->data, message->length);
		fl_free_message_data(message);
		message = NULL;
	}
	pthread_exit(0);
}

static void s_login_server_loop()
{
	int i;
	int clientfd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(struct sockaddr);
	uint32_t session = 0;

	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	//start main loop
	fd_set readset;
	int maxfd = -1, selectn = 0;
	struct timeval tv;

	//create read thread
	for (i=0;i<2;++i)
	{
		pthread_create(&tid, &attr, s_read_thread, NULL);
	}

	//create work thread
	for (i=0;i<2;++i)
	{
		pthread_create(&tid, &attr, s_work_thread, NULL);
	}

	fl_log(0,"starting login loop");
	while (true)
	{
		if (0 == s_run_state) break;
		maxfd = -1;
		FD_ZERO(&readset);
		FD_SET(s_listen_fd, &readset);
		maxfd = s_listen_fd > maxfd ? s_listen_fd : maxfd;

		//close client in queue first
//		s_do_real_close_client();

		for (i=0;i<MAX_CLIENT_CONNECTIONS;++i)
		{
			if (s_connections[i].GetSockfd() > maxfd)
			{
				maxfd = s_connections[i].GetSockfd();
				FD_SET(s_connections[i].GetSockfd(), &readset);
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
				i = s_conn_bit_record.GetUnSetBitPosition();
				if (-1 != i)
				{
					s_conn_bit_record.SetBit(i);
					fl_log(1,"[login]:Accept client %d,client index = %d\n", clientfd, i);
					s_connections[i].SetSocketInfo(clientfd, ++session);
					s_connections[i].SetAddrInfo(&client_addr);
					if (session > 0XFFFFFFFE)
					{
						session = 0;
					}
				}
			}
			--selectn;
		}

		if (selectn > 0)
		{
			for (i=0;i<MAX_CLIENT_CONNECTIONS;++i)
			{
				if (-1 == s_connections[i].GetSockfd()) continue;
				if (clientfd == s_connections[i].GetSockfd()) continue;
				if (FD_ISSET(s_connections[i].GetSockfd(), &readset))
				{
					if (false == s_read_bit_record.IsSetBit(i))
					{
						s_read_bit_record.SetBit(i);
						s_read_msg_queue.push_message(i);
					}
					--selectn;
				}
				if (0 == selectn) break;
			}
		}

	}   //end of while(true)
}  //server main loop end
