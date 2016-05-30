#ifndef SRC_ENGINE_LOGIN_H_
#define SRC_ENGINE_LOGIN_H_

#include <sys/socket.h>
#include <arpa/inet.h>

struct login_connection
{
	int fd;  //连接fd
	struct sockaddr_in addr; /* 连接的地址信息 */
	int last_time; /* 上一条消息是什么时候接收到的 */
};

void fl_start_login_server();



#endif /* SRC_ENGINE_LOGIN_H_ */
