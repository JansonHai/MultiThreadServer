#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "gate.h"
#include "logger.h"
#include "buffer.h"
#include "ByteArray.h"
#include "BitRecord.h"
#include "MsgQueue.h"
#include "envirment.h"

static int s_listen_fd = -1;
static int s_gate_server_pid = -1;
static int s_run_state = 0;
static int MAX_CLIENT_CONNECTIONS = 2048;
static int MAX_MESSAGE_LENGTH = 2097152;

static class BitRecord s_conn_bit_record;
static class BitRecord s_read_bit_record;
static class MsgQueue<int> s_read_msg_queue;
static class MsgQueue<struct fl_message_data *> s_work_msg_queue;
static class fl_connection * s_connections;
static pthread_mutex_t s_close_mutex;

static void s_gate_server_loop();
static void s_recv_data_callback(struct fl_message_data * message);
