#ifndef SRC_NET_H_
#define SRC_NET_H_

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

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
