#ifndef SRC_NET_H_
#define SRC_NET_H_

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#define MAX_BUFF_LEN 4096

struct fl_connection_info {
    int fd; /* socket连接 */
    struct sockaddr_in addr; /* 连接的地址信息 */
    char text[MAX_BUFF_LEN]; /* 接收的消息缓冲 */
    int text_end; /* 接收消息缓冲的尾指针 */
    int text_start; /* 接收消息缓冲的头指针 */
    int last_time; /* 上一条消息是什么时候接收到的 */
    struct timeval latency; /* 客户端本地时间和服务器本地时间的差值 */
    struct timeval last_confirm_time; /* 上一次验证的时间 */
    short is_confirmed; /* 该连接是否通过验证过 */
    int ping_num; /* 该客户端到服务器端的ping值 */
    int ping_ticker; /* 多少个IO周期处理更新一次ping值 */
    int message_length; /* 发送缓冲消息长度 */
    char message_buf[MAX_BUFF_LEN]; /* 发送缓冲区 */
    int iflags; /* 该连接的状态 */
};


struct fl_message_data
{
	int fd;
	int session;
	char * data;
	uint32_t length;
	uint32_t readLength;
	bool isReadFinish;
};

struct fl_net_message_queue
{
	struct fl_message_data * current_data;
};

void fl_net_init();

bool fl_send_message(int fd,const char * data,uint32_t length);
void fl_malloc_message_data(struct fl_message_data * &message, int fd = -1, int session = -1);
void fl_reset_message_data(struct fl_message_data * &message, int fd = -1, int session = -1);
void fl_drop_message_data(struct fl_message_data * &message);
void fl_free_message_data(struct fl_message_data * &message);

void fl_push_read_fd_index(int index);
int fl_pop_read_fd_index();
void fl_reset_read_fd_index(int index);

void fl_push_handle_message(struct fl_message_data * message);
void fl_pop_handle_message(struct fl_message_data * &message);

void fl_close_client_fd(int fd);
void fl_close_client_index(int index);
void fl_do_real_client_close();

#endif /* SRC_NET_H_ */
