#ifndef SRC_NET_H_
#define SRC_NET_H_

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <netinet/in.h>

union _Int32
{
	int i;
	char c[4];
};

struct fl_message_data
{
	int index;
	int fd;
	uint32_t session;
	union _Int32 header;
	char headerReadLength;
	char * data;
	int length;
	int readLength;
	bool isReadFinish;
	bool isDropMessage;
};

void fl_malloc_message_data(struct fl_message_data * &message, int index = -1, int fd = -1, int session = 0);
void fl_reset_message_data(struct fl_message_data * &message, int index = -1, int fd = -1, int session = 0);
void fl_drop_message_data(struct fl_message_data * &message);
void fl_free_message_data(struct fl_message_data * &message);


class fl_connection
{
public:
	typedef void (*CallBack)(struct fl_message_data * message);
private:
	int index;
	int sockfd;
	uint32_t session;
	int message_max_length;
	struct sockaddr_in addr;
	pthread_rwlock_t rwlock;
	struct fl_message_data * recv_message;
	CallBack m_callback;
public:
	fl_connection();
	~fl_connection();
	void SetIndex(int index);
	void SetSocketInfo(int sockfd, uint32_t session);
	void SetMaxMessageLength(int length);
	void SetRecvCallBack(CallBack callback);
	void SetAddrInfo(struct sockaddr_in * addr);
	int GetSockfd();
	int GetSession();
	bool Recv();
	bool Send(const char * data, int len);
	void Close();
};


struct fl_backgate_data
{
	int fd;
	uint32_t session;
	bool isAuth;

};

#endif /* SRC_NET_H_ */
