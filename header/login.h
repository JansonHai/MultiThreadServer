#ifndef SRC_ENGINE_LOGIN_H_
#define SRC_ENGINE_LOGIN_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>
#include "net.h"
#include "ByteArray.h"

struct login_connection
{
	int fd;  //连接fd
	uint32_t session;  //连接的session
	bool isCloseing; //标记是否准备要关闭的了
	pthread_mutex_t mutex;  //互斥锁
	struct sockaddr_in addr; /* 连接的地址信息 */
	struct fl_message_data * recv_message;  //接收缓冲区
};

bool fl_start_login_server();
void fl_stop_login_server();
void fl_login_send_message_to_client(int index, uint32_t session, const char * data, int length);
bool fl_start_login_watchdog_server();
void fl_stop_login_watchdog_server();


#endif /* SRC_ENGINE_LOGIN_H_ */
