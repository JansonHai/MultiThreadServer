#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "MsgQueue.h"
#include "gamelogic.h"

static void * s_work_thread(void * arg);
static int s_run_state = 0;

static class MsgQueue<struct fl_gamelogic_ctx *> s_work_msg_queue;

#define MAX_CTX_LENGTH 25600
static struct fl_gamelogic_ctx * s_ctxs;
static int s_ctx_pos = 0;
static pthread_mutex_t s_dispatch_message_lock;

void fl_start_gamelogic()
{
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	s_run_state = 1;

	//create work thread
	for (int i=0;i<4;++i)
	{
		pthread_create(&tid, &attr, s_work_thread, NULL);
	}

	s_ctxs = (struct fl_gamelogic_ctx *)malloc(MAX_CTX_LENGTH * sizeof(struct fl_gamelogic_ctx));
	pthread_mutex_init(&s_dispatch_message_lock, NULL);

}

void fl_stop_gamelogic()
{
	free(s_ctxs);
	pthread_mutex_destroy(&s_dispatch_message_lock);
}

void fl_dispatch_message(class fl_connection * conn, struct fl_message_data * message)
{
	pthread_mutex_lock(&s_dispatch_message_lock);
	struct fl_gamelogic_ctx * ctx = &s_ctxs[s_ctx_pos++];
	if (s_ctx_pos >= MAX_CTX_LENGTH) s_ctx_pos = 0;
	ctx->conn = conn;
	ctx->message = message;
	s_work_msg_queue.push_message(ctx);
	pthread_mutex_unlock(&s_dispatch_message_lock);
}


static void * s_work_thread(void * arg)
{
//	int index, readLeft, readn, length;
	bool result;
	struct fl_gamelogic_ctx * ctx;
	struct fl_message_data * message;
	class fl_connection * conn;
	ReadByteArray * readByteArray;
	while (true)
	{
		if (0 == s_run_state) break;
		message = NULL;
		result = s_work_msg_queue.pop_message(ctx);
		if (false == result || NULL == ctx)
		{
			usleep(10000);  //10ms;
			continue;
		}

		conn = ctx->conn;
		message = ctx->message;
		if (conn->GetSession() != message->session)
		{
			fl_free_message_data(message);
			message = NULL;
			continue;
		}

		//handle
		readByteArray = new ReadByteArray();
		readByteArray->SetReadContent(message->data, message->length);
		fl_free_message_data(message);
		delete ctx;
		message = NULL;
	}
	pthread_exit(0);
}
