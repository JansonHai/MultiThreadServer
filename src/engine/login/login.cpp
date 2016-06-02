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
#include "BitRecord.h"
#include "MsgQueue.h"

static int s_listen_fd = -1;
static uint32_t s_session = 0;
static int s_login_server_pid = -1;
static int s_run_state = 0;
static int MAX_CLIENT_CONNECTIONS = 2048;

static class BitRecord s_conn_bit_record;
static class BitRecord s_read_bit_record;
static class BitRecord s_close_bit_record;
static class MsgQueue<int> s_read_msg_queue;
static class MsgQueue<int> s_close_msg_queue;
static class MsgQueue<struct fl_message_data *> s_work_msg_queue;
static struct login_connection * s_connections;
static pthread_mutex_t s_close_mutex;

static void s_login_server_loop();

void fl_stop_login_server()
{
	s_run_state = 0;
	usleep(1000000 * 5);  //5S
	free(s_connections);
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
	s_connections = (struct login_connection * )malloc(MAX_CLIENT_CONNECTIONS * sizeof(struct login_connection));
	for (int i=0;i<MAX_CLIENT_CONNECTIONS;++i)
	{
		s_connections[i].fd = -1;
		s_connections[i].session = 0;
		memset(&s_connections[i].addr,0,sizeof(s_connections[i].addr));
		s_connections[i].recv_message = NULL;
		s_connections[i].isCloseing = false;
		pthread_mutex_init(&s_connections[i].mutex, NULL);
	}

	//init bit flag to mark connection
	s_conn_bit_record.SetBitsLength(MAX_CLIENT_CONNECTIONS);
	s_read_bit_record.SetBitsLength(MAX_CLIENT_CONNECTIONS);
	s_close_bit_record.SetBitsLength(MAX_CLIENT_CONNECTIONS);
	pthread_mutex_init(&s_close_mutex, NULL);

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


void fl_close_client_conn(int index)
{
	pthread_mutex_lock(&s_close_mutex);
	if (true == s_conn_bit_record.IsSetBit(index) && false == s_close_bit_record.IsSetBit(index))
	{
		s_close_msg_queue.push_message(index);
		s_close_bit_record.SetBit(index);
	}
	pthread_mutex_unlock(&s_close_mutex);
}

static void s_do_real_close_client()
{
	pthread_mutex_lock(&s_close_mutex);
	int index = -1;
	struct login_connection * conn = NULL;
	while (true == s_close_msg_queue.pop_message(index))
	{
		conn = &s_connections[index];
		if (NULL == conn || -1 == conn->fd) continue;
		close(conn->fd);
		conn->fd = -1;
		conn->session = -1;
		s_conn_bit_record.ResetBit(index);
	}
	pthread_mutex_unlock(&s_close_mutex);
}

void fl_send_client_message()
{

}

static void * s_read_thread(void * arg)
{
	int index, readLeft, readn, length;
	bool result;
	struct login_connection * conn;
	while (true)
	{
		_read_thread_loop_begin:
		if (s_run_state != 0) break;
		result = s_read_msg_queue.pop_message(index);
		if (false == result)
		{
			usleep(10000);  //10ms;
			continue;
		}
		conn = &s_connections[index];
		pthread_mutex_lock(&conn->mutex);
		if (-1 == conn->fd)
		{
			goto _read_thread_loop_end;
		}

		if (NULL == conn->recv_message)
		{
			fl_malloc_message_data(conn->recv_message, index, conn->fd, conn->session);
		}

		if (conn->session != conn->recv_message->session)
		{
			fl_reset_message_data(conn->recv_message, index, conn->fd, conn->session);
		}

		if (4 != conn->recv_message->headerReadLength)
		{
			while (conn->recv_message->headerReadLength != 4)
			{
				readLeft = 4 - conn->recv_message->headerReadLength;
				readn = recv(conn->fd, &conn->recv_message->header.c + conn->recv_message->headerReadLength, readLeft, MSG_DONTWAIT | MSG_NOSIGNAL);
				if (0 == readn)
				{
					fl_log(2,"client %d,session %u close\n", conn->fd, conn->session);
					fl_close_client_conn(index);
					goto _read_thread_loop_end;
				}
				else if (-1 == readn)
				{
					if (EAGAIN == errno || EINTR == errno)
					{
						goto _read_thread_loop_end;
					}
					else
					{
						fl_log(2,"client %d,session %u read error,errno: %d\n",conn->fd, conn->session, errno);
						goto _read_thread_loop_end;
					}
				}
				conn->recv_message->headerReadLength += readn;
			}
			length = ntohl(conn->recv_message->header.i);
			if (length <= MAX_MESSAGE_LENGTH)
			{
				conn->recv_message->length = length;
				conn->recv_message->data = (char *)malloc(length * sizeof(char));
				conn->recv_message->readLength = 0;
				conn->recv_message->isReadFinish = false;
				conn->recv_message->isDropMessage = false;
			}
			else
			{
				fl_log(2,"client %d send message too large,length : %d, this message will drop\n", conn->fd, length);
				conn->recv_message->length = length;
				conn->recv_message->data = (char *)malloc(1024 * sizeof(char));
				conn->recv_message->readLength = 0;
				conn->recv_message->isReadFinish = false;
				conn->recv_message->isDropMessage = true;
			}
		}

		if (conn->recv_message->length > 0)
		{
			while (conn->recv_message->readLength < conn->recv_message->length)
			{
				if (false == conn->recv_message->isDropMessage)
				{
					readLeft = conn->recv_message->length - conn->recv_message->readLength;
					readn = recv(conn->fd, conn->recv_message->data + conn->recv_message->readLength, readLeft, MSG_DONTWAIT | MSG_NOSIGNAL);
				}
				else
				{
					readn = recv(conn->fd, conn->recv_message->data, 1024, MSG_DONTWAIT | MSG_NOSIGNAL);
				}
				if (0 == readn)
				{
					fl_log(2,"client %d,session %u close\n", conn->fd, conn->session);
					fl_close_client_conn(index);
					goto _read_thread_loop_end;
				}
				else if (-1 == readn)
				{
					if (EAGAIN == errno || EINTR == errno)
					{
						goto _read_thread_loop_end;
					}
					else
					{
						fl_log(2,"client %d,session %u read error,errno: %d\n",conn->fd, conn->session, errno);
						goto _read_thread_loop_end;
					}
				}
				conn->recv_message->readLength += readn;
				if (conn->recv_message->readLength == conn->recv_message->length)
				{
					conn->recv_message->isReadFinish = true;
					if (false == conn->recv_message->isDropMessage)
					{
						//push message to work thread
						s_work_msg_queue.push_message(conn->recv_message);
						conn->recv_message = NULL;
					}
					else
					{
						//drop this message
						fl_drop_message_data(conn->recv_message);
					}
					goto _read_thread_loop_end;
				}
				if (true == conn->recv_message->isDropMessage)
				{
					goto _read_thread_loop_end;
				}
			}
		}

		_read_thread_loop_end:
		s_read_bit_record.ResetBit(index);
		pthread_mutex_unlock(&conn->mutex);
	}
	pthread_exit(0);
}

static void * s_work_thread(void * arg)
{
	int index, readLeft, readn, length;
	bool result;
	struct fl_message_data * message;
	struct login_connection * conn;
	ReadByteArray * readByteArray;
	while (true)
	{
		if (s_run_state != 0) break;
		message = NULL;
		result = s_work_msg_queue.pop_message(message);
		if (false == result)
		{
			usleep(10000);  //10ms;
			continue;
		}

		conn = &s_connections[message->index];
		if (conn->session != message->session)
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

static void * s_write_thread(void * arg)
{
	while (true)
	{
		if (s_run_state != 0) break;
		usleep(10000);  //10ms;
	}
	pthread_exit(0);
}

static void s_login_server_loop()
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

	//create read thread
	pthread_create(&tid, &attr, s_read_thread, NULL);

	//create work thread
	pthread_create(&tid, &attr, s_work_thread, NULL);

	//create write thread
	pthread_create(&tid, &attr, s_write_thread, NULL);

	while (true)
	{
		if (s_run_state != 0) break;
		maxfd = -1;
		FD_ZERO(&readset);
		FD_SET(s_listen_fd, &readset);
		maxfd = s_listen_fd > maxfd ? s_listen_fd : maxfd;

		//close client first
		s_do_real_close_client();

		for (i=0;i<MAX_CLIENT_CONNECTIONS;++i)
		{
			if (s_connections[i].fd > maxfd)
			{
				maxfd = s_connections[i].fd;
				FD_SET(s_connections[i].fd, &readset);
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
					fl_log(1,"Accept client %d,client index = %d\n", clientfd, i);
					s_connections[i].session = ++s_session;
					s_connections[i].fd= clientfd;
					s_connections[i].addr = client_addr;
					if (s_session > 0XFFFFFFFE)
					{
						s_session = 0;
					}
				}
			}
			--selectn;
		}

		if (selectn > 0)
		{
			for (i=0;i<MAX_CLIENT_CONNECTIONS;++i)
			{
				if (-1 == s_connections[i].fd) continue;
				if (clientfd == s_connections[i].fd) continue;
				if (FD_ISSET(s_connections[i].fd, &readset))
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
