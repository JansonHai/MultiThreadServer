#ifndef SRC_NET_H_
#define SRC_NET_H_

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

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

void fl_net_init();

void fl_malloc_message_data(struct fl_message_data * &message, int index = -1, int fd = -1, int session = 0);
void fl_reset_message_data(struct fl_message_data * &message, int index = -1, int fd = -1, int session = 0);
void fl_drop_message_data(struct fl_message_data * &message);
void fl_free_message_data(struct fl_message_data * &message);

#endif /* SRC_NET_H_ */
