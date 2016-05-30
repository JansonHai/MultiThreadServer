#ifndef SRC_HANDLER_H_
#define SRC_HANDLER_H_

#include <unistd.h>
#include <mysql/mysql.h>

struct handle_info
{
	pid_t pid;
	int socketfd;
	struct sockaddr_in client_addr;  //ipv4
	struct sockaddr_in6 client_addr6;  //ipv6
	char ip[64];
	uint16_t port;
};

struct handle_context
{
	struct handle_info client;
	MYSQL * mysql_conn;
};

void fl_start_child_handle(struct handle_info client);
void fl_start_child_logic(struct handle_context * context);

#endif /* SRC_HANDLER_H_ */
