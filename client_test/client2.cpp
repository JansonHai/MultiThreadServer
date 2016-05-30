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

int SERVER_PORT  = 6666;

#define MAXBUFLEN 1024

int send_failed = 0;
int recv_failed = 0;

int loggerfd;

time_t start;
time_t end;
unsigned int count = 0;
#define THREAD_LENGTH 4
pthread_t tid[THREAD_LENGTH];

static void signal_handler(int s)
{
	if (s == SIGINT)
	{
		end = time(NULL);
		time_t diff = end - start;
		double dt = (double)diff;
		double rs = count / dt;
		printf("\nuse time=%0.2fs, total request=%d, avarge is %0.2f r/s\n",dt,count,rs);
		printf("send failed = %d, recv failed = %d\n", send_failed, recv_failed);
		printf("send failed rate = %0.2f%%, recv failed rate = %0.2f%%\n", send_failed * 100.0f / count, recv_failed * 100.0f / count);
		for (int i=0;i<THREAD_LENGTH;++i)
		{
			pthread_cancel(tid[i]);
		}
		close(loggerfd);
		exit(0);
	}
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void client_log(int level, const char * msg, ...);

void * run(void * arg)
{
	int sockfd;
	struct sockaddr_in server_addr;
	char buf[MAXBUFLEN];
	int m_count;

	do
	{
		m_count = ++count;
		sockfd = socket(AF_INET,SOCK_STREAM,0);
		if (-1 == sockfd)
		{
			client_log(2,"[%u] create socket failed, errno: %d\n", m_count, errno);
			continue;
		}

		memset(&server_addr,0,sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(SERVER_PORT);
		server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

		if (-1 == connect(sockfd,(struct sockaddr*)&server_addr, sizeof(struct sockaddr)))
		{
			client_log(2,"[%u] connect server failed, errno:%d\n", m_count, errno);
			continue;
		}

		char str[10];
		strcpy(str,"hello");
		int len = strlen(str) + 1;
		int netlength = htonl(len);
		int n;

		do
		{
			len = strlen(str) + 1;
			netlength = htonl(len);
			n = send(sockfd, &netlength, 4, MSG_NOSIGNAL);
			if (n < 0)
			{
				client_log(2,"[%u] send data length failed, errno:%d\n", m_count, errno);
				++send_failed;
				break;
			}
			else if (n > 0)
			{
				n = send(sockfd, str, len, MSG_NOSIGNAL);
				if (n < 0)
				{
					client_log(2,"[%u] send data failed, errno:%d\n", m_count, errno);
					++send_failed;
					break;
				}
			}
			else
			{
				client_log(2,"[%u] server close ..\n", m_count);
				break;
			}
			len = 0;
			n = recv(sockfd, &netlength, 4, MSG_NOSIGNAL);
			if (n < 0)
			{
				client_log(2,"[%u] recv data length failed, errno: %d\n", m_count, errno);
				++recv_failed;
				break;;
			}
			else if (n > 0)
			{
				len = ntohl(netlength);
				n = recv(sockfd, buf, len, MSG_NOSIGNAL);
				if (n < 0)
				{
					client_log(2,"[%u] recv data failed, errno: %d\n", m_count, errno);
					++recv_failed;
					break;
				}
			}
			else
			{
				client_log(2,"[%u] server close .., errno: %d\n", m_count, errno);
				break;
			}
			printf("[%u] Server Message: %s\n", m_count, buf);
		} while (false);
		close(sockfd);
		printf("[%u] client connect close\n", m_count);
	} while (false);
	client_log(2,"[%u] thread unknow exit...\n", m_count);
	pthread_exit(0);
}

int main(int argc,char * argv[])
{
	signal(SIGINT,signal_handler);

    if (2 == argc)
    {
    	SERVER_PORT = atoi(argv[1]);
    }

    loggerfd = open("./log/client.log", O_WRONLY | O_APPEND | O_CREAT, 0644);
    start = time(NULL);

    for (int i=0;i<THREAD_LENGTH;++i)
    {
    	pthread_create(&tid[i],NULL,run,NULL);
    }

    for (int i=0;i<THREAD_LENGTH;++i)
    {
    	pthread_join(tid[i],NULL);
    }

    return 0;
}

void client_log(int level, const char * msg, ...)
{
	if (-1 == loggerfd) return;
	char levelStr[20];
	switch(level)
	{
	case 0:
		strcpy(levelStr, "[INFO]");
		break;
	case 1:
		strcpy(levelStr, "[WARNING]");
		break;
	case 2:
		strcpy(levelStr, "[ERROR]");
		break;
	case 8888:
		strcpy(levelStr, "[DEBUG]");
		break;
	case 9999:
		strcpy(levelStr, "[EXIT]");
		break;
	default:
		strcpy(levelStr, "[INFO]");
		break;
	}


	char vbuf[2048];
	va_list vlist;
	va_start(vlist, msg);
	vsnprintf(vbuf, sizeof(vbuf), msg, vlist);
	va_end(vlist);


	time_t ct;
	struct tm * ctm;
	ct = time(NULL);
	ctm = localtime(&ct);

	char logBuf[2048];
	snprintf(logBuf, sizeof(logBuf), "%s %02d:%02d:%02d : %s",levelStr,ctm->tm_hour,ctm->tm_min,ctm->tm_sec,vbuf);
	int len = strlen(logBuf);
	if ('\n' != logBuf[len-1])
	{
		logBuf[len++] = '\n';
		logBuf[len++] = 0;
	}
	write(loggerfd,logBuf,strlen(logBuf));

}
