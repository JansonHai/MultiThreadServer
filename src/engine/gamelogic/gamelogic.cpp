#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ByteArray.h"
#include "MsgQueue.h"
#include "gamelogic.h"

extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

static void * s_work_thread(void * arg);
static int s_run_state = 0;

static class MsgQueue<struct fl_gamelogic_ctx *> s_work_msg_queue;

#define MAX_CTX_LENGTH 25600
#define WORK_THREAD_NUM 4
static struct fl_gamelogic_ctx * s_ctxs;
static int s_ctx_pos = 0;
static pthread_mutex_t s_dispatch_message_lock;
static pthread_mutex_t s_work_thread_lock[WORK_THREAD_NUM];
static pthread_cond_t s_work_cond_lock[WORK_THREAD_NUM];
static bool s_work_thread_busy[WORK_THREAD_NUM];
static int s_work_id[WORK_THREAD_NUM];

static lua_State * Lua;

void fl_start_gamelogic()
{
	pthread_mutex_init(&s_dispatch_message_lock, NULL);
	for (int i=0;i<WORK_THREAD_NUM;++i)
	{
		pthread_mutex_init(&s_work_thread_lock[i], NULL);
		pthread_cond_init(&s_work_cond_lock[i], NULL);
		s_work_thread_busy[i] = false;
	}

	s_ctxs = (struct fl_gamelogic_ctx *)malloc(MAX_CTX_LENGTH * sizeof(struct fl_gamelogic_ctx));

	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	s_run_state = 1;

	//create work thread
	for (int i=0;i<WORK_THREAD_NUM;++i)
	{
		s_work_id[i] = i;
		pthread_create(&tid, &attr, s_work_thread, (void*)(&s_work_id[i]));
	}

	Lua = luaL_newstate();
	luaL_openlibs(Lua);
}

void fl_stop_gamelogic()
{
	s_run_state = 0;
	struct fl_gamelogic_ctx * ctx;
	while (s_work_msg_queue.pop_message(ctx))
	{
		fl_free_message_data(ctx->message);
	}
	free(s_ctxs);
	pthread_mutex_destroy(&s_dispatch_message_lock);
	for (int i=0;i<WORK_THREAD_NUM;++i)
	{
		pthread_mutex_destroy(&s_work_thread_lock[i]);
		pthread_cond_destroy(&s_work_cond_lock[i]);
	}
	lua_close(Lua);
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

	//wake up work thread
	for (int i=0;i<WORK_THREAD_NUM;++i)
	{
		if (false == s_work_thread_busy[i])
		{
			pthread_mutex_lock(&s_work_thread_lock[i]);
			s_work_thread_busy[i] = true;
			pthread_cond_signal(&s_work_cond_lock[i]);
			pthread_mutex_unlock(&s_work_thread_lock[i]);
			break;
		}
	}
}


static void * s_work_thread(void * arg)
{
	int work_id = *((int*)arg);
	bool result;
	struct fl_gamelogic_ctx * ctx;
	struct fl_message_data * message;
	class fl_connection * conn;
	ReadByteArray readByteArray;
	while (true)
	{
		if (0 == s_run_state) break;
		message = NULL;
		result = s_work_msg_queue.pop_message(ctx);
		if (false == result || NULL == ctx)
		{
			pthread_mutex_lock(&s_work_thread_lock[work_id]);
			s_work_thread_busy[work_id] = false;
			pthread_cond_wait(&s_work_cond_lock[work_id], &s_work_thread_lock[work_id]);
			pthread_mutex_unlock(&s_work_thread_lock[work_id]);
			continue;
		}
		s_work_thread_busy[work_id] = true;
		conn = ctx->conn;
		message = ctx->message;
		if (conn->GetSession() != message->session)
		{
			fl_free_message_data(message);
			message = NULL;
			continue;
		}

		//handle
		readByteArray.SetReadContent(message->data, message->length);
		readByteArray.ReleaseBuffer();
		fl_free_message_data(message);
		message = NULL;
	}
	pthread_exit(0);
}
