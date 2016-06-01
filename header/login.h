#ifndef SRC_ENGINE_LOGIN_H_
#define SRC_ENGINE_LOGIN_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>

struct login_connection
{
	int fd;  //连接fd
	uint32_t session;
	struct sockaddr_in addr; /* 连接的地址信息 */
	char readBuf[2048];
	char writeBuf[2048];
};

void fl_start_login_server();
void fl_stop_login_server();


#endif /* SRC_ENGINE_LOGIN_H_ */
