#ifndef SRC_NET_H_
#define SRC_NET_H_

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#define MAX_BUFF_LEN 4096

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

struct fl_net_message_queue
{
	struct fl_message_data * current_data;
};

void fl_net_init();

void fl_malloc_message_data(struct fl_message_data * &message, int index = -1, int fd = -1, int session = 0);
void fl_reset_message_data(struct fl_message_data * &message, int index = -1, int fd = -1, int session = 0);
void fl_drop_message_data(struct fl_message_data * &message);
void fl_free_message_data(struct fl_message_data * &message);

//bool fl_send_message(int fd,const char * data,int length);
//void fl_close_client_index(int index);

//void fl_push_read_fd_index(int index);
//int fl_pop_read_fd_index();
//void fl_reset_read_fd_index(int index);
//
//void fl_push_handle_message(struct fl_message_data * message);
//void fl_pop_handle_message(struct fl_message_data * &message);
//
//void fl_close_client_fd(int fd);

#endif /* SRC_NET_H_ */
