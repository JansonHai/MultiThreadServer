#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include "net.h"
#include "buffer.h"
#include "ByteArray.h"

char SERVER_IP[] = "127.0.0.1";
int SERVER_PORT  = 6701;

#define MAXBUFLEN 1024

int send_failed = 0;
int recv_failed = 0;

int loggerfd;

time_t start;
time_t end;
unsigned int count = 0;
#define THREAD_LENGTH 4
pthread_t tid[THREAD_LENGTH];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc,char * argv[])
{

    int sockfd;
	struct sockaddr_in server_addr;
	char buf[MAXBUFLEN];

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (-1 == sockfd)
	{
		printf("create socket failed, errno: %d\n", errno);
		return 0;
	}

	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	if (-1 == connect(sockfd,(struct sockaddr*)&server_addr, sizeof(struct sockaddr)))
	{
		printf("connect server failed, errno:%d\n", errno);
		close(sockfd);
		return 0;
	}

	class fl_connection conn;
	conn.SetIndex(0);
	conn.SetSocketInfo(sockfd,1);

	char cmd[4096];

	fl_init_buffer();
	WriteByteArray wb;
	wb.InitBuffer(2048);

	while (true)
	{
		printf("Please input command:\n");
		scanf("%s",cmd);
		wb.ResetWrite();
		wb.WriteString(cmd,strlen(cmd));
		conn.Send(wb.GetBuffer(),wb.GetArraySize());
	}

	conn.Close();

    return 0;
}
