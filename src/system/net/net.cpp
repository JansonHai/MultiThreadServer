#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include "net.h"
#include "exit.h"
#include "logger.h"
#include "global.h"


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
	message->header.i = 0;
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
	message->header.i = 0;
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
	message->header.i = 0;
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

fl_connection::fl_connection()
{
	index = -1;
	sockfd = -1;
	session = 0;
	message_max_length = 131072;   //128K
	recv_message = NULL;
	m_callback = NULL;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	pthread_rwlock_init(&rwlock, NULL);
}

fl_connection::~fl_connection()
{
	free(rbuf);
	free(wbuf);
	pthread_rwlock_destroy(&rwlock);
}

void fl_connection::SetIndex(int index)
{
	this->index = index;
}

void fl_connection::SetSocketInfo(int sockfd, uint32_t session)
{
	pthread_rwlock_wrlock(&rwlock);
	this->sockfd = sockfd;
	this->session = session;
	struct timeval timeout={5,0};  //5s
	setsockopt(this->sockfd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
	setsockopt(this->sockfd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
	pthread_rwlock_unlock(&rwlock);
}

void fl_connection::SetMaxMessageLength(int length)
{
	message_max_length = length;
}

void fl_connection::SetRecvCallBack(CallBack callback)
{
	this->m_callback = callback;
}

bool fl_connection::Recv()
{
	pthread_rwlock_rdlock(&rwlock);
	int index, readLeft, readn, length, tryContinueTime;
	bool ret;
	if (-1 == sockfd)
	{
		ret = false;
		goto _read_end;
	}

	if (NULL == recv_message)
	{
		fl_malloc_message_data(recv_message, index, sockfd, session);
	}

	if (session != recv_message->session)
	{
		fl_reset_message_data(recv_message, index, sockfd, session);
	}

	if (4 != recv_message->headerReadLength)
	{
		tryContinueTime = 0;
		while (recv_message->headerReadLength != 4)
		{
			readLeft = 4 - recv_message->headerReadLength;
			readn = recv(sockfd, &recv_message->header.c + recv_message->headerReadLength, readLeft, MSG_DONTWAIT | MSG_NOSIGNAL);

			if (0 == readn)
			{
				ret = false;
				fl_log(2,"recv header failed, client %d,session %u close\n", sockfd, session);
				goto _read_end;
			}
			else if (-1 == readn)
			{
				if (EWOULDBLOCK == errno || EAGAIN == errno || EINTR == errno)
				{
					if (tryContinueTime < 3)
					{
						++tryContinueTime;
						continue;
					}
					else
					{
						ret = true;
						goto _read_thread_loop_end;
					}
				}
				else
				{
					ret = false;
					fl_log(2,"client %d,session %u recv header error,errno: %d\n",sockfd, session, errno);
					goto _read_thread_loop_end;
				}
			}
			recv_message->headerReadLength += readn;
		}

		length = (int)ntohl(recv_message->header.i);
		if (length <= message_max_length)
		{
			recv_message->length = length;
			recv_message->data = (char *)malloc(length * sizeof(char));
			recv_message->readLength = 0;
			recv_message->isReadFinish = false;
			recv_message->isDropMessage = false;
		}
		else
		{
			fl_log(2,"client %d,session %u send message too large,message length: %d,this message will drop\n", sockfd, session, length);
			recv_message->length = length;
			recv_message->data = (char *)malloc(1024 * sizeof(char));
			recv_message->readLength = 0;
			recv_message->isReadFinish = false;
			recv_message->isDropMessage = true;
		}
	}

	if (recv_message->length > 0)
	{
		tryContinueTime = 0;
		while (recv_message->readLength < recv_message->length)
		{
			if (false == recv_message->isDropMessage)
			{
				readLeft = recv_message->length - recv_message->readLength;
				readn = recv(sockfd, recv_message->data + recv_message->readLength, readLeft, MSG_DONTWAIT | MSG_NOSIGNAL);
			}
			else
			{
				readn = recv(sockfd, recv_message->data, 1024, MSG_DONTWAIT | MSG_NOSIGNAL);
			}

			if (0 == readn)
			{
				ret = false;
				fl_log(2,"recv data failed,client %d,session %u close\n", sockfd, session);
				goto _read_end;
			}
			else if (-1 == readn)
			{
				if (EWOULDBLOCK == errno || EAGAIN == errno || EINTR == errno)
				{
					if (true == recv_message->isDropMessage)
					{
						ret = true;
						goto _read_end;
					}
					else
					{
						if (tryContinueTime < 3)
						{
							++tryContinueTime;
							continue;
						}
						else
						{
							ret = true;
							goto _read_end;
						}
					}
				}
				else
				{
					ret = false;
					fl_log(2,"client %d,session %u recv data error,errno: %d\n", sockfd, session, errno);
					goto _read_end;
				}
			}

			recv_message->readLength += readn;
			if (recv_message->readLength == recv_message->length)
			{
				recv_message->isReadFinish = true;
				if (false == recv_message->isDropMessage)
				{
					//push message to work thread
					if (NULL != m_callback)
					{
						m_callback(recv_message);
					}
					else
					{
						fl_drop_message_data(recv_message);
					}
					recv_message = NULL;
				}
				else
				{
					//drop this message
					fl_drop_message_data(recv_message);
				}
				ret = true;
				goto _read_end;
			}
			if (true == recv_message->isDropMessage)
			{
				ret = true;
				goto _read_end;
			}
		}
	}

	ret = true;

	_read_end:
	pthread_rwlock_unlock(&rwlock);
	return ret;
}

bool fl_connection::Send(char * data, int len)
{
	pthread_rwlock_rdlock(&rwlock);

	int sendn,headerSended;
	bool ret;

	if (-1 == sockfd)
	{
		ret = false;
		goto _write_end;
	}

	union _Int32 header;
	header.i = (int)htonl(len);

	headerSended = 0;
	while (headerSended < 4)
	{
		sendn = send(sockfd, header.c + headerSended, 4 - headerSended, MSG_NOSIGNAL);
		if (0 == sendn)
		{
			ret = false;
			fl_log(2,"send header failed, client %d,session %u closed\n", sockfd, session);
			goto _read_end;
		}
		else if (-1 == sendn)
		{
			if (EWOULDBLOCK == errno || EAGAIN == errno || EINTR == errno)
			{
				continue;
			}
			else
			{
				ret = false;
				fl_log(2,"client %d,session %u send header error,errno: %d\n", sockfd, session, errno);
				goto _read_thread_loop_end;
			}
		}
		headerSended += sendn;
	}

	while (len > 0)
	{
		sendn = send(sockfd, data, len, MSG_NOSIGNAL);
		if (0 == sendn)
		{
			ret = false;
			fl_log(2,"send data failed, client %d,session %u closed\n", sockfd, session);
			goto _read_end;
		}
		else if (-1 == sendn)
		{
			if (EWOULDBLOCK == errno || EAGAIN == errno || EINTR == errno)
			{
				continue;
			}
			else
			{
				ret = false;
				fl_log(2,"client %d,session %u send data error,errno: %d\n", sockfd, session, errno);
				goto _read_thread_loop_end;
			}
		}
		data += sendn;
		len -= sendn;
	}

	ret = true;

	_write_end:
	pthread_rwlock_unlock(&rwlock);
	return ret;
}

void fl_connection::SetAddrInfo(struct sockaddr_in * addr)
{
	memcpy(&this->addr, addr, sizeof(struct sockaddr_in));
}

int fl_connection::GetSockfd()
{
	return sockfd;
}

int fl_connection::GetSession()
{
	return session;
}

void fl_connection::Close()
{
	pthread_rwlock_wrlock(&rwlock);
	close(sockfd);
	sockfd = -1;
	session = 0;
	pthread_rwlock_unlock(&rwlock);
}



